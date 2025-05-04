#include "CC.h"
#include <nch/cpp-utils/color.h>
#include <nch/cpp-utils/fs-utils.h>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/input.h>
#include "Constants.h"
#include "Main.h"
#include "Resources.h"
#include "TileProperties.h"

using namespace nch;

CC::CC()
{
    if(initialized) { return; }
    initialized = true;

    TileProperties::create();

    initUIs();
    lossMsgNeedsUpdate = true;

    #ifdef NDEBUG
        setGamestate(SPLASH_SCREEN);
    #else
        setGamestate(TITLE);
    #endif
}
CC::~CC()
{
    if(!initialized) { return; }
    initialized = false;

    delete btnFreeplay;
    delete btnQuit;
    delete world;
}

void CC::tick()
{
    if(!initialized) return;

    if(Input::keyDownTime(SDLK_F10)==1) {
        reloadResources();
    }

    /* Tick world if it exists */
    if(world!=nullptr) {
        //Screen focus
        if(screen.isFocused())  {
            SDL_ShowCursor(SDL_DISABLE);
        } else {
            SDL_ShowCursor(SDL_ENABLE);
        }
        if(Input::keyDownTime(SDLK_F11)==1 || Input::keyDownTime(SDLK_ESCAPE)==1) {
            if(world->getState()!=World::LOST) {
                screen.setFocused(false);
                SDL_WarpMouseInWindow(Main::getWindow(), Main::getWidth()/2, Main::getHeight()/2);
                setGamestate(LEVEL_SELECT);
                return;
            }
        }

        world->tick();
        switch(world->getState()) {
            case World::LOST: {
                screen.setFocused(false);

                ticksWhileLost++;
    
                if(ticksWhileLost>100) {
                    if(Input::isMouseDown(1)) {
                        enterLevel();
                        lossMsgNeedsUpdate = true;
                    }
                    if(Input::isMouseDown(3) || Input::keyDownTime(SDLK_ESCAPE)==1) {
                        setGamestate(LEVEL_SELECT);
                        lossMsgNeedsUpdate = true;
                    }
                }
            } break;
            case World::WON: {
                enterLevel(true);
            } break;
            default: {
                ticksWhileLost = 0;
            } break;
        }
    }

    /* Miscellaneous */
    if(lossMsgNeedsUpdate) {
        lossMsgNeedsUpdate = false;
        std::srand(time(NULL));
        int idx = std::rand()%Constants::lossMsgs.size();
        lossTxt.setText(Constants::lossMsgs.at(idx));
    }

    /* Tick UIs depending on gamestate */
    switch(gamestate) {
        case SPLASH_SCREEN: {
            if(ticksOnSplashScreen==0) {
                Resources::getSFXByID(Resources::SFX_impact)->playback();
            }
            ticksOnSplashScreen++;
            if(ticksOnSplashScreen>40*5) {
                setGamestate(TITLE);
            }
        } break;
        case TITLE: {
            btnFreeplay->tick(screen);
            btnQuit->tick(screen);

            if(btnFreeplay->popClicked()) setGamestate(LEVEL_SELECT);
            if(btnQuit->popClicked()) setGamestate(QUIT);
        } break;
        case LEVEL_SELECT: {
            btnBack->tick(screen);
            if(btnBack->popClicked()) setGamestate(TITLE);

            btnPrev->tick(screen);
            if(btnPrev->popClicked()) lvlSel.incSectionIdx(-1);
            
            btnNext->tick(screen);
            if(btnNext->popClicked()) lvlSel.incSectionIdx(1);

            lvlSel.tick(screen);
            int clickedLvlIdx = lvlSel.popClickedLvlIdx();
            if(clickedLvlIdx!=-1) {
                selectedLvlSection = lvlSel.getSectionName();
                selectedLvlIndex = clickedLvlIdx;
                setGamestate(WORLD);
            }
        } break;
    }
}

void CC::draw()
{
    if(!initialized) return;
    SDL_Renderer* rend = Main::getRenderer();
    if(!screen.wasInitialized()) screen.init(rend); //Init screen if needed

    /* Draw world if it exists */
    bool currentlyLost = false;
    if(world!=nullptr) {
        screen.update(world->getMapBounds());
        world->draw(screen);
        if(world->getState()==World::LOST) {
            currentlyLost = true;
        }
    } else {
        if(!Main::isFullscreen()) {
            screen.update(Box2<double>(0, 0, 1920-192, 1080-108));
        } else {
            screen.update(Box2<double>(0, 0, Main::getWidth(), Main::getHeight()));
        }
    }

    /* Draw UIs depending on gamestate */
    switch(gamestate) {
        case SPLASH_SCREEN: {
            drawSplashScreen();
        } break;
        case TITLE: {
            drawTitleBackground();
            //Logo texture
            MMTexture* logo = Resources::getTextureByID(Resources::ID::TEX_logo);
            if(logo==nullptr) return;
            //Logo render
            Rect logoDst = Rect( screen.getUnscaledWidth()/2-logo->getWidth(), 200+40, logo->getWidth()*2, logo->getHeight()*2 );
            screen.transformRect(logoDst);
            logo->renderCopy(rend, NULL, &logoDst.r);

            //Buttons
            btnFreeplay->setPos(Vec2<int>(screen.getUnscaledWidth()/2-btnFreeplay->getUnscaledWidth()/2, 200+280));
            btnFreeplay->draw(screen);

            btnQuit->setPos(Vec2<int>(screen.getUnscaledWidth()/2-btnQuit->getUnscaledWidth()/2, 200+360));
            btnQuit->draw(screen);
        } break;
        case LEVEL_SELECT: {
            drawTitleBackground();
            
            btnBack->setPos(Vec2<int>(screen.getUnscaledWidth()/2-btnBack->getUnscaledWidth()/2, screen.getUnscaledHeight()-btnBack->getUnscaledHeight()-24));
            btnBack->draw(screen);

            btnBack->setPos(Vec2<int>(screen.getUnscaledWidth()/2-btnBack->getUnscaledWidth()/2, screen.getUnscaledHeight()-btnBack->getUnscaledHeight()-24));
            btnBack->draw(screen);
            
            btnPrev->setPos(Vec2<int>(screen.getUnscaledWidth()/2-btnPrev->getUnscaledWidth()*1.5, 24));
            btnPrev->draw(screen);

            btnNext->setPos(Vec2<int>(screen.getUnscaledWidth()/2+btnNext->getUnscaledWidth()*0.5, 24));
            btnNext->draw(screen);

            lvlSel.draw(screen);
        } break;
    }

    /* Screen borders */
    screen.drawBorders();
    //Draw text that may appear above screen borders
    if(currentlyLost) drawLossScreen();
    drawTimer();
}

void CC::drawCursor()
{
    if(!initialized) return;

    if(world!=nullptr)
        world->drawCursor(screen);
}

void CC::drawLossScreen()
{
    SDL_Renderer* rend = Main::getRenderer();

    //Transparency
    int alpha = ticksWhileLost*32;
    if(alpha>192) {
        alpha = 192;
    }
    //Rectangle
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, alpha);
    SDL_RenderFillRect(rend, NULL);
    
    if(alpha>0) {
        double scale = 1.0;
        lossTxt.setScale(scale);
        while(lossTxt.getWidth()>Main::getWidth()*2/3) {
            scale = scale-0.05;
            lossTxt.setScale(scale);
        }
        int lmw = lossTxt.getWidth();
        int lmh = lossTxt.getHeight()/1.25;
        int lmy = (Main::getHeight()/1.25)/2-lmh/2;
        lossTxt.draw(Main::getWidth()/2-lmw/2, lmy);
        lossTxt.setTextColor(Color(255, 255, 255, alpha+63));

        scale = 4;
        continueTxt.setScale(scale);
        while(continueTxt.getWidth()>Main::getWidth()/3) {
            scale = scale-0.1;
            continueTxt.setScale(scale);
        }
        int tcw = continueTxt.getWidth();
        continueTxt.draw(Main::getWidth()/2-tcw/2, lmy+lossTxt.getHeight()+scale*16);
    }
}

void CC::drawTitleBackground()
{
    SDL_Renderer* rend = Main::getRenderer();

    //Background
    Rect bgDst = Rect(0, 0, screen.getUnscaledWidth(), screen.getUnscaledHeight());
    screen.transformRect(bgDst);
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderFillRect(rend, &bgDst.r);

    //Tiles
    Color base0 = Color(255, 192, 192);
    Color base1 = Color(192, 192, 255);
    for(int ix = 0; ix<screen.getUnscaledWidth()/64.+3; ix++)
    for(int iy = 0; iy<screen.getUnscaledHeight()/64.; iy++) {
        int tileX = ix*64-((Timer::getTicks()/10)%128);
        Rect tileDst = Rect(tileX, iy*64, 64, 64);
        screen.transformRect(tileDst);
        if((ix+iy)%2==0)    { SDL_SetRenderDrawColor(rend, base0.r, base0.g, base0.b, base0.a ); }
        else                { SDL_SetRenderDrawColor(rend, base1.r, base1.g, base1.b, base1.a ); }
        SDL_RenderFillRect(rend, &tileDst.r);
    }
}

void CC::drawSplashScreen()
{
    SDL_Renderer* rend = Main::getRenderer();
    MMTexture* mtex = Resources::getTextureByID(Resources::TEX_ootws);
    if(mtex==nullptr) return;

    int bteW = 1024;
    int bteH = 512;

    //Get rectangle for company logo
    Rect dst0 = Rect(
        screen.getUnscaledWidth()/2-bteW/2,
        screen.getUnscaledHeight()/2-bteH/2,
        bteW,
        bteH
    );

    //Get location of splash text under logo
    splashTxt.setScale(screen.getScale());
    Rect dst1 = Rect(
        screen.getUnscaledWidth()/2-splashTxt.getWidth()/screen.getScale()/2,
        dst0.r.y+dst0.r.h,
        0, 0
    );

    /* Draw elements */
    //Get alpha
    int a = ticksOnSplashScreen*3;
    if(a>255) a = 255;
    //Draw logo
    screen.transformRect(dst0);
    mtex->setAlphaMod(a);
    mtex->setBlendMode(SDL_BLENDMODE_BLEND);
    mtex->renderCopy(rend, NULL, &dst0.r);
    //Draw splash text
    screen.transformRect(dst1);
    splashTxt.setTextColor(Color(255, 255, 255, a));
    splashTxt.draw(dst1.r.x, dst1.r.y);
    /**/
}

void CC::drawTimer() {
    if(world==nullptr) return;
    uint64_t tx = world->getNumTicksActive();
    uint64_t parTx = world->getParTicks();

    /* Set timer string */ {
        //Components
        uint64_t millis = 25*(tx%40);
        uint64_t secs = tx/40;
        uint64_t mins = secs/60;
        std::stringstream ss;

        //Minutes
        if(mins<10) ss << "0";
        ss << mins << ":";
        //Seconds
        if(secs<10) ss << "0";
        ss << secs << ".";
        //Milliseconds
        if(millis<100) ss << "0";
        if(millis<10) ss << "0";
        ss << millis;
        timerTxt.setText(ss.str());
    }


    //Draw timer background and string
    double s = screen.getScale();
    SDL_Renderer* rend = Main::getRenderer();
    Rect dst0b(
        Main::getWidth()-timerTxt.getWidth(),
        Main::getHeight()-timerTxt.getHeight(),
        timerTxt.getWidth(),
        timerTxt.getHeight()
    );
    Rect dst0a = dst0b;

    dst0a.r.x -= s*16*4;
    dst0a.r.w += s*16*4;
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 127);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(rend, &dst0a.r);

    timerTxt.setScale(s);
    timerTxt.draw(dst0b.x1(), dst0b.y1());

    /* Draw stars */ {
        //Get texture
        MMTexture* guis = Resources::getTextureByID(Resources::TEX_guis);
        if(guis==nullptr) return;
        guis->setBlendMode(SDL_BLENDMODE_BLEND);

        //Get star score
        double starScore = world->getCurrentStarScore();

        //Draw full stars
        for(int i = 0; i<std::ceil(starScore); i++) {
            Rect src1(127, 1, 9, 9);
            if(std::ceil(starScore)!=starScore && i==std::ceil(starScore)-1) {
                src1 = Rect(117, 1, 9, 9);
            }
            if(i>std::ceil(starScore)) {
                src1 = Rect(117, 11, 9, 9);
            }

            Rect dst1(
                dst0b.x1() - s*16*4,
                dst0b.y1()+i*s*9*4,
                s*9*4,
                s*9*4
            );
            guis->renderCopy(rend, &src1.r, &dst1.r);
        }
    }


}

int CC::getGamestate() {
    return gamestate;
}

void CC::reloadResources()
{
    SDL_Renderer* rend = Main::getRenderer();
    Resources::reloadAll();

    //Reload buttons and text objects
    lossMsgNeedsUpdate = true;
    initUIs();
}

void CC::setGamestate(int gs)
{
    gamestate = gs;

    unloadWorld();
    SDL_ShowCursor(SDL_ENABLE);
    
    switch(gamestate) {
        case SPLASH_SCREEN: {
            ticksOnSplashScreen = 0;
        } break;
        case LEVEL_SELECT: {
            lvlSel.setSectionIdx(lvlSel.getSectionIdx());
        } break;
        case WORLD: {
            enterLevel();
        } break;
    }
}

void CC::unloadWorld()
{
    if(world!=nullptr) { 
        delete world;
        world = nullptr;
    }
}

void CC::enterLevel(bool advanceLevel)
{
    unloadWorld();
    if(selectedLvlSection=="???null???") {
        setGamestate(LEVEL_SELECT);
        return;
    }

    //Use current level index OR (if advancing) increment it until we find a valid level (max of 16 iterations)
    if(!advanceLevel) selectedLvlIndex--;
    bool foundValidLevel = false;
    int numSearches = 16;
    for(int i = 1; i<=numSearches; i++) {
        std::stringstream path;
        path << Main::getBasePath() << "data/" << selectedLvlSection << "/lvl_" << (selectedLvlIndex+i);
        
        FsUtils::setLogWarnings(false);
        std::string fullPath = FsUtils::getPathWithInferredExtension(path.str());
        FsUtils::setLogWarnings(true);
        if(fullPath=="?null?") {
            Log::warnv(__PRETTY_FUNCTION__, "looking for next index...", "Couldn't find file matching \"%s.*\"", path.str().c_str());
        } else {
            selectedLvlIndex = selectedLvlIndex+i;
            foundValidLevel = true;
            break;
        }
    }
    if(!foundValidLevel) {
        setGamestate(LEVEL_SELECT);
        Log::warnv(__PRETTY_FUNCTION__, "stopping world load and returning to level selection.", "Couldn't find any valid level after %d iterations", numSearches);
        return;
    }

    screen.setFocused(true);
    world = new World();
    std::stringstream levelPath;
    levelPath << selectedLvlSection << "/lvl_" << selectedLvlIndex;
    world->loadLevel(levelPath.str());
    world->setThemeBySection(selectedLvlSection);
}
void CC::enterLevel() {
    enterLevel(false);
}

void CC::initUIs()
{
    /* Buttons */ {
        if(btnFreeplay!=nullptr) { delete btnFreeplay; btnFreeplay = nullptr; }
        btnFreeplay = new Button(Button::ID::freeplay);
        btnFreeplay->setText("Freeplay");
    
        if(btnQuit!=nullptr) { delete btnQuit; btnQuit = nullptr; }
        btnQuit = new Button(Button::ID::quit);
        btnQuit->setText("Quit");
    
        if(btnBack!=nullptr) { delete btnBack; btnBack = nullptr; }
        btnBack = new Button(Button::ID::back_to_title);
        btnBack->setText("Back to Title Screen");
    
        if(btnNext!=nullptr) { delete btnNext; btnNext = nullptr; }
        btnNext = new Button(Button::ID::next);
        btnNext->setWidth(8*16); btnNext->setText("Next >");
    
        if(btnPrev!=nullptr) { delete btnPrev; btnPrev = nullptr; }
        btnPrev = new Button(Button::ID::prev);
        btnPrev->setWidth(8*16); btnPrev->setText("< Prev.");
    }

    /* Text objects */ {
        splashTxt.init(Main::getRenderer(), Resources::ttfBte100);
        splashTxt.setShadowing(false);
        splashTxt.setTextColor(Color(255, 255, 255));
        splashTxt.setText("presents...");
    
        lossTxt.init(Main::getRenderer(), Resources::ttfOswaldBold96);
    
        continueTxt.init(Main::getRenderer(), Resources::ttfOswald48);
        continueTxt.setText("Left-click to restart; right-click to quit.");
    
        timerTxt.init(Main::getRenderer(), Resources::ttfBte100);
        timerTxt.setShadowing(false);
        timerTxt.setTextColor(Color(255, 255, 255));

        lvlSel.reloadHeaderTxt();
    }
}