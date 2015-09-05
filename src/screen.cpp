#include "screen.h"

Screen *Screen::instance = NULL;

Screen *Screen::getInstance() {
	if (instance == NULL) {
		instance = new Screen();
	}
	return instance;
}

void Screen::cleanUpInstance() {
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

Screen::Screen():
	sdlInitErrorOccured(false),
	fullscreen(false),
	rect_num(0)
{
	// initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        sdlInitErrorOccured = true;
    }
	if(!sdlInitErrorOccured && TTF_Init() == -1) {
		printf("TTF initialization failed: %s\n", TTF_GetError());
        sdlInitErrorOccured = true;
	}
	if (!sdlInitErrorOccured) {
		screen_surface = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE);
		if(screen_surface == 0) {
			printf("Setting video mode failed: %s\n",SDL_GetError());
			sdlInitErrorOccured = true;
		}
	}
	if (!sdlInitErrorOccured) {
		SDL_WM_SetCaption("Pacman", "");
	}
	atexit(Screen::cleanUpInstance);
}

Screen::~Screen() {
	TTF_Quit();
	SDL_Quit();
}

void Screen::AddUpdateRects(int x, int y, int w, int h) {
    if (rect_num >= 200)
        return;  // prevent index out of bounds problems
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > this->screen_surface->w)
        w = this->screen_surface->w - x;
    if (y + h > this->screen_surface->h)
        h = this->screen_surface->h - y;
    if (w <= 0 || h <= 0)
        return;
	rects[rect_num].x = (short int)x;
	rects[rect_num].y = (short int)y;
	rects[rect_num].w = (short int)w;
	rects[rect_num].h = (short int)h;
	rect_num++;
}

void Screen::Refresh() {
	SDL_UpdateRects(this->screen_surface, this->rect_num, this->rects);
	this->rect_num = 0;
}

void Screen::draw_dynamic_content(SDL_Surface *surface, int x, int y) {
	SDL_Rect dest;
	dest.x = (short int)x;
	dest.y = (short int)y;
	SDL_BlitSurface(surface, NULL, this->screen_surface, &dest);
	this->AddUpdateRects(dest.x, dest.y, surface->w + 10, surface->h);
}

void Screen::draw(SDL_Surface* graphic, int offset_x, int offset_y) {
    if (0 == offset_x && 0 == offset_y) {
        SDL_BlitSurface(graphic, NULL, screen_surface, NULL);
    } else {
        SDL_Rect position;
        position.x = (short int)offset_x;
        position.y = (short int)offset_y;
        SDL_BlitSurface(graphic, NULL, screen_surface, &position);
    }
}

void Screen::setFullscreen(bool fs) {
    if (fs == fullscreen)
        return;  // the desired mode already has been activated, so do nothing
    SDL_Surface* newScreen;
    if(fs)
        newScreen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | SDL_FULLSCREEN);
	else
        newScreen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE);
    if (NULL != newScreen) {  // successful? NULL indicates failure
        this->screen_surface = newScreen;  // take it, but do not dispose of the old screen (says SDL documentation)
        this->AddUpdateRects(0, 0, this->screen_surface->w, this->screen_surface->h);
        // no Refresh() here, because at this moment nothing has been drawn to the new screen
        fullscreen = fs;
    }
}

void Screen::drawHorizontalLine(int x1, int x2, int y, Uint8 r, Uint8 g, Uint8 b) {
	if (SDL_MUSTLOCK(this->screen_surface))
		SDL_LockSurface(this->screen_surface);
	Uint8* p;
	for (int i = x1; i <= x2; ++i) {
		p = (Uint8*) this->screen_surface->pixels + (y * this->screen_surface->pitch) + (i * sizeof(Uint8) * 3);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		p[0] = r; p[1] = g; p[2] = b;
#else
		p[0] = b; p[1] = g; p[2] = r;
#endif
	}
	if (SDL_MUSTLOCK(this->screen_surface))
		SDL_UnlockSurface(this->screen_surface);
}

void Screen::drawVerticalLine(int x, int y1, int y2, Uint8 r, Uint8 g, Uint8 b) {
	if (SDL_MUSTLOCK(this->screen_surface))
		SDL_LockSurface(this->screen_surface);
	Uint8* p;
	for (int i = y1; i <= y2; ++i) {
		p = (Uint8*) this->screen_surface->pixels + (i * this->screen_surface->pitch) + (x * sizeof(Uint8) * 3);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		p[0] = r; p[1] = g; p[2] = b;
#else
		p[0] = b; p[1] = g; p[2] = r;
#endif
	}
	if (SDL_MUSTLOCK(this->screen_surface))
		SDL_UnlockSurface(this->screen_surface);
}

SDL_Surface *Screen::LoadSurface(const char *filename, int transparent_color) {
	SDL_Surface *surface, *temp;
	temp = IMG_Load(filename);
	if(!temp) {
		printf("Unable to load image: %s\n", IMG_GetError());
		exit(-1);
	}
	if(transparent_color != -1)
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(temp->format, (uint8_t)transparent_color, (uint8_t)transparent_color, (uint8_t)transparent_color));
	surface = SDL_DisplayFormat(temp);
	if(surface == NULL) {
		printf("Unable to convert image to display format: %s\n", SDL_GetError());
                exit(EXIT_FAILURE);
        }
    SDL_FreeSurface(temp);
    return surface;
}

void Screen::clear() {
	SDL_Rect rect {0, 0, 640,480};
	this->fillRect(&rect, 0, 0, 0);
}

void Screen::fillRect(SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b) {
	SDL_FillRect(screen_surface, rect, SDL_MapRGB(screen_surface->format, r, g, b));
}
