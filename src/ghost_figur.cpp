#include "ghost_figur.h"
#include <stdlib.h>

Ghost::Ghost(int init_x, int init_y, int init_intelligence, 
             Direction init_direction, int init_up_down, int ghost_ident,
             Screen *screen, Labyrinth *labyrinth, Pacman *pacman):
	Figur(init_x, init_y, GHOSTS_V_NORMAL, screen, labyrinth),
	its_leader(0),
	initial_intelligence(init_intelligence),
	initial_direction(init_direction),
	initial_up_down(init_up_down),
	ghost_ident(ghost_ident)
{
	direction = init_direction;
	intelligence = init_intelligence;
	up_down = init_up_down;
	this->set_hunter(GHOST);
	this->pacman = pacman;

	// Surfaces
	if(ghost_ident == BLINKY) {
		ghost_1 = LoadSurface("/usr/local/share/pacman/gfx/blinky_1.png", 255);
		ghost_2 = LoadSurface("/usr/local/share/pacman/gfx/blinky_2.png", 255);
	}
	else if(ghost_ident == PINKY) {
		ghost_1 = LoadSurface("/usr/local/share/pacman/gfx/pinky_1.png", 255);
		ghost_2 = LoadSurface("/usr/local/share/pacman/gfx/pinky_2.png", 255);
	}
	else if(ghost_ident == INKY) {
		ghost_1 = LoadSurface("/usr/local/share/pacman/gfx/inky_1.png", 255);
		ghost_2 = LoadSurface("/usr/local/share/pacman/gfx/inky_2.png", 255);
	}
	else if(ghost_ident == CLYDE) {
		ghost_1 = LoadSurface("/usr/local/share/pacman/gfx/clyde_1.png", 255);
		ghost_2 = LoadSurface("/usr/local/share/pacman/gfx/clyde_2.png", 255);
	}
	augen_0 = LoadSurface("/usr/local/share/pacman/gfx/augen_0.png", 0);
	augen_1 = LoadSurface("/usr/local/share/pacman/gfx/augen_1.png", 0);
	augen_2 = LoadSurface("/usr/local/share/pacman/gfx/augen_2.png", 0);
	augen_3 = LoadSurface("/usr/local/share/pacman/gfx/augen_3.png", 0);
	escape_1 = LoadSurface("/usr/local/share/pacman/gfx/escaping_ghost_1.png", 255);
	escape_2 = LoadSurface("/usr/local/share/pacman/gfx/escaping_ghost_2.png", 255);
	escape_white_1 = LoadSurface("/usr/local/share/pacman/gfx/escaping_ghost_white_1.png", 0);
	escape_white_2 = LoadSurface("/usr/local/share/pacman/gfx/escaping_ghost_white_2.png", 0);
	ar_ghost[0] = ghost_1;
	ar_ghost[1] = ghost_2;
	ar_ghost[2] = escape_white_1;
	ar_ghost[3] = escape_white_2;
	num_animation_frames = 2;
	idx_animation = 1;
	this->ghost_sf = ar_ghost[idx_animation];
}

Ghost::~Ghost() {
	SDL_FreeSurface(ghost_1);
	SDL_FreeSurface(ghost_2);
	SDL_FreeSurface(augen_0);
	SDL_FreeSurface(augen_1);
	SDL_FreeSurface(augen_2);
	SDL_FreeSurface(augen_3);
	SDL_FreeSurface(escape_1);
	SDL_FreeSurface(escape_2);
	SDL_FreeSurface(escape_white_1);
	SDL_FreeSurface(escape_white_2);
}

void Ghost::draw() {
	if (this->visible) {
		if (this->get_hunter() != NONE)
			this->screen->draw(this->ghost_sf, this->x, this->y);
		if (this->get_hunter() != PACMAN) {
			switch(this->get_direction()) {
				case LEFT:
					this->screen->draw(augen_0, this->x, this->y);
					break;
				case UP: 
					this->screen->draw(augen_1, this->x, this->y);
					break;
				case RIGHT:
					this->screen->draw(augen_2, this->x, this->y);
					break;
				case DOWN: 
					this->screen->draw(augen_3, this->x, this->y);
					break;
				default:
					this->screen->draw(augen_0, this->x, this->y);
					break;
			}
		}
	}
}

void Ghost::animation() {
	idx_animation = (idx_animation + 1) % num_animation_frames;
	this->ghost_sf = ar_ghost[idx_animation];
}

void Ghost::set_leader(bool leader) {
	its_leader = leader;
}

void Ghost::set_leader() {
	int i;
	for(i = 0;i < 4; i++) {
		if(ghost_array[i]->getGhostIdent() == this->getGhostIdent())
			ghost_array[i]->set_leader(true);
		else
			ghost_array[i]->set_leader(false);
	}
}

void Ghost::move_dir(int ms, int direction, int max_links, int max_oben, int max_rechts, int max_unten) {
	if(direction == LEFT)
		move_left(ms, max_links);
	if(direction == UP)
		move_up(ms, max_oben);
	if(direction == RIGHT)
		move_right(ms, max_rechts);
	if(direction == DOWN)
		move_down(ms, max_unten);	
	if(its_leader) {
		if(this->was_moving())
			was_moving_leader = 1;
		else
			was_moving_leader = 0;
	}	
}

void Ghost::move(int ms) {
	this->move_on_rails(ms, this->labyrinth->number_rails(), labyrinth->array_rails);
	if (last_x != x || last_y != y)
		this->addUpdateRect();
}

int Ghost::direction_to_point(int target_x, int target_y) {
	int dx = abs(target_x - this->x);
	int dy = abs(target_y - this->y);
	if (dx > dy) {
		return (target_x - this->x > 0) ? RIGHT : LEFT;
	} else {
		return (target_y - this->y > 0) ? DOWN : UP;
	}
}

int Ghost::alternative_direction_to_point(int target_x, int target_y) {
	int dx = abs(target_x - this->x);
	int dy = abs(target_y - this->y);
	if (dx > dy) {
		// actually, here, a horizontal direction would be the best one, but what if it's not available?
		return (target_y - this->y > 0) ? 3 /*down*/ : 1 /*up*/;
	} else {
		// actually vertical, alternative will be horizontal
		return (target_x - this->x > 0) ? 2 /*to the right*/ : 0 /*to the left*/;
	}
}

int Ghost::get_intelligence() const {
	return intelligence;
}

int Ghost::choose_direction(Direction * sammel_richtung, int richtung_pacman, int sammel_counter, int intelligence) {
	int idx_dir_pacman = -1;
	int zufallswert;
	int idx_direction;
	int castle_direction;
	int castle_x = 310, castle_y = 190;

	if(sammel_counter == 1)
		return sammel_richtung[0];

	if (this->get_hunter() == NONE) {
		// try to return to the ghost castle
		castle_direction = this->direction_to_point(castle_x, castle_y);
		for (int i = 0; i < sammel_counter; ++i) {
			if (sammel_richtung[i] == castle_direction)
				return castle_direction;
		}
		// try an alternative direction
		castle_direction = this->alternative_direction_to_point(castle_x, castle_y);
		for (int i = 0; i < sammel_counter; ++i) {
			if (sammel_richtung[i] == castle_direction)
				return castle_direction;
		}
		// if neither the exact nor the alternative direction can be taken, then choose randomly
		zufallswert = (rand() % 100 + 1);
		return sammel_richtung[zufallswert%sammel_counter];
	}

	for(int i = 0; i < sammel_counter; ++i) {
		if(sammel_richtung[i] == richtung_pacman){
			idx_dir_pacman = i;
		}
	}
	zufallswert = (rand() % 100 + 1);
	if (this->get_hunter() == PACMAN) {
		if (idx_dir_pacman == -1 || zufallswert > intelligence) {
			return sammel_richtung[zufallswert%sammel_counter];
		} else {
			idx_direction = zufallswert % (sammel_counter-1);
			if (idx_direction == idx_dir_pacman)
				++idx_direction;
			return sammel_richtung[idx_direction];
		}
	} else {
		if(idx_dir_pacman != -1) {
			if(zufallswert <= intelligence) 
				return richtung_pacman;
		}
		return sammel_richtung[zufallswert%sammel_counter];
	}
}

void Ghost::move_on_rails(int ms, int anz_schienen, Rail **ar_s) {
	int i;
	int richtung_ghost = this->get_direction();
	Direction new_direction;
	int richtung_pacman = this->direction_to_point(pacman->x, pacman->y);
	Direction sammel_richtung[3];
	int sammel_counter = 0;
	int max_links = 0;                              // maximum to the left (otherwise, the ghost will leave the rails)
	int max_oben = 0;                               // maximum up
	int max_rechts = 10000;                         // maximum to the right
	int max_unten = 10000;                          // maximum down
	int bridge;

	for(i = 0; i <= anz_schienen - 1; i++) {
		// first, check the tunnel
		if((richtung_ghost != RIGHT) && (this->x <= 100) && (this->y == 215)) {
	 		this->x = 515;
	 		this->cur_x = this->x << 10;
	 		richtung_ghost = LEFT;
	 		break;
		}
		if((richtung_ghost != LEFT) && (this->x >= 515) && (this->y == 215)) {
	 		this->x = 100;
	 		this->cur_x = this->x << 10;
	 		richtung_ghost = RIGHT;
	 		break;
		}

		if (i==89 && (this->get_hunter()==NONE || richtung_ghost==UP)) {
			bridge = 1;  // for returning eyes-only ghosts, open the door to the castle
		} else if (i==86 && richtung_ghost==RIGHT) {
			bridge = 1;  // allow to leave the castle, but do not return to this rail
		} else if (i==87 && richtung_ghost==LEFT) {
			bridge = 1;  // allow to leave the castle, but do not return to this rail
		} else {
			bridge = 0;
		}

		if (richtung_ghost == LEFT) {
			if (this->x == ar_s[i]->x1 && this->x == ar_s[i]->x2) {  // vertical rail
				if (this->y > ar_s[i]->y1 && this->y <= ar_s[i]->y2) {
					sammel_richtung[sammel_counter] = UP;
					++sammel_counter;
					max_oben = ar_s[i]->y1;
				}
				if (this->y < ar_s[i]->y2 && this->y >= ar_s[i]->y1 - bridge) {
					sammel_richtung[sammel_counter] = DOWN;
					++sammel_counter;
					max_unten = ar_s[i]->y2;
				}
			}
			if (this->y == ar_s[i]->y1 && this->y == ar_s[i]->y2 && this->x > ar_s[i]->x1-bridge && this->x <= ar_s[i]->x2) {  // horizontal rail, space to the left
				sammel_richtung[sammel_counter] = LEFT;
				++sammel_counter;
				max_links = ar_s[i]->x1-bridge;
			}
			if(sammel_counter == 3)
				break;
		} else if (richtung_ghost == UP) {
			if (this->y == ar_s[i]->y1 && this->y == ar_s[i]->y2 && !this->up_down) {  // horizontal rail
				if (this->x > ar_s[i]->x1 && this->x <= ar_s[i]->x2) {
					sammel_richtung[sammel_counter] = LEFT;
					++sammel_counter;
					max_links = ar_s[i]->x1;
				}
				if (this->x < ar_s[i]->x2 && this->x >= ar_s[i]->x1) {
					sammel_richtung[sammel_counter] = RIGHT;
					++sammel_counter;
					max_rechts = ar_s[i]->x2;
				}
			}
			if (this->x == ar_s[i]->x1 && this->x == ar_s[i]->x2 && this->y > ar_s[i]->y1-bridge && this->y <= ar_s[i]->y2) {  // vertical rail, space above
				sammel_richtung[sammel_counter] = UP;
				++sammel_counter;
				max_oben = ar_s[i]->y1 - bridge;
			}
			if(sammel_counter == 3)
				break;
		} else if (richtung_ghost == RIGHT) {
			if (this->x == ar_s[i]->x1 && this->x == ar_s[i]->x2) {  // vertical rail
				if (this->y > ar_s[i]->y1 && this->y <= ar_s[i]->y2) {
					sammel_richtung[sammel_counter] = UP;
					++sammel_counter;
					max_oben = ar_s[i]->y1;
				}
				if (this->y < ar_s[i]->y2 && this->y >= ar_s[i]->y1 - bridge) {
					sammel_richtung[sammel_counter] = DOWN;
					++sammel_counter;
					max_unten = ar_s[i]->y2;
				}
			}
			if (this->y == ar_s[i]->y1 && this->y == ar_s[i]->y2 && this->x < ar_s[i]->x2+bridge && this->x >= ar_s[i]->x1) {  // horizontal rail, space to the right
				sammel_richtung[sammel_counter] = RIGHT;
				++sammel_counter;
				max_rechts = ar_s[i]->x2+bridge;
			}
			if(sammel_counter == 3)
				break;
		} else if (richtung_ghost == DOWN) {
			if ((i == 89) && (this->x == ar_s[i]->x2) && (this->y >= ar_s[i]->y2)) {
				// special case: at the bottom of the castle's vertical center rail, the ghost must invert it's direction (i.e. go up again)
				sammel_richtung[0] = UP;
				sammel_counter = 1;
				max_oben = ar_s[i]->y1;
				// also make an eyes-only ghost normal again
				if (this->get_hunter() == NONE)
					this->set_hunter(GHOST);
				break;  // no more directions allowed
			}
			if (this->y == ar_s[i]->y1 && this->y == ar_s[i]->y2 && !this->up_down) {  // horizontal rail
				if (this->x > ar_s[i]->x1 && this->x <= ar_s[i]->x2) {
					sammel_richtung[sammel_counter] = LEFT;
					++sammel_counter;
					max_links = ar_s[i]->x1;
				}
				if (this->x < ar_s[i]->x2 && this->x >= ar_s[i]->x1) {
					sammel_richtung[sammel_counter] = RIGHT;
					++sammel_counter;
					max_rechts = ar_s[i]->x2;
				}
			}
			if (this->x == ar_s[i]->x1 && this->x == ar_s[i]->x2 && this->y < ar_s[i]->y2 && this->y >= ar_s[i]->y1) {  // vertical rail, space below
				sammel_richtung[sammel_counter] = DOWN;
				++sammel_counter;
				max_unten = ar_s[i]->y2;
			}
			if(sammel_counter == 3)
				break;
		}
	}
	if(sammel_counter == 0 && this->up_down) {  // 0 occurs when the ghost reaches the end of the rail, and up_down is still active
		if(richtung_ghost == UP) {
			sammel_richtung[sammel_counter] = DOWN;
			sammel_counter++;
			this->up_down--;
		} else if(richtung_ghost == DOWN) {
			sammel_richtung[sammel_counter] = UP;
			sammel_counter++;
			this->up_down--;
		}
	}
	if (sammel_counter > 0) {
		richtung_ghost = choose_direction(sammel_richtung, richtung_pacman, sammel_counter, this->get_intelligence());
		this->move_dir(ms, richtung_ghost, max_links, max_oben, max_rechts, max_unten);
	}
}

void Ghost::reset() {
	x = initial_x;
	y = initial_y;
	dx = initial_v;
	dy = initial_v;
	last_x = initial_x;
	last_y = initial_y;
	cur_x = initial_x << 10;
	cur_y = initial_y << 10;
	direction = initial_direction;
	intelligence = initial_intelligence;
	up_down = initial_up_down;
	this->set_hunter(GHOST);
}

void Ghost::addUpdateRect() {
	screen->AddUpdateRects(least(x,last_x), least(y,last_y), ghost_sf->w + abs(x-last_x), ghost_sf->h + abs(y-last_y));
}

SDL_Surface* Ghost::get_Surface() const {
	return ghost_sf;
}

Figur::Hunter Ghost::get_hunter() const {
	return hunter;
}

void Ghost::set_hunter(Hunter hunter) {
	if(hunter == PACMAN) {
		this->set_speed(GHOSTS_V_SLOW);
		ar_ghost[0] = escape_1;
		ar_ghost[1] = escape_2;
	} else {
		this->set_speed(GHOSTS_V_NORMAL);
		ar_ghost[0] = ghost_1;
		ar_ghost[1] = ghost_2;
	}
	num_animation_frames = 2;
	this->hunter = hunter;
}

bool Ghost::touched() {
	if(this->get_hunter() == PACMAN) {
		// ghost has been eaten by pacman
		this->hunter = NONE;
		this->set_speed(GHOSTS_V_NORMAL);
		this->set_leader();
		this->setVisibility(0);
		this->pacman->setVisibility(0);
		this->labyrinth->addBonusScore(this->x + (ghost_sf->w >> 1), this->y + (ghost_sf->h >> 1));
		this->labyrinth->increaseBonusStage();
		this->labyrinth->sleep(400);
	}
	if(this->get_hunter() == NONE)
		return false;  // no problem for pacman
	return true;
}

void Ghost::blink() {
	if(this->get_hunter() == PACMAN) {
		num_animation_frames = 4;
	}
}

void Ghost::setGhostArray(Ghost **ghost_array) {
	this->ghost_array = ghost_array;
}

Ghost::Ghosts Ghost::getGhostIdent() const {
	return (Ghosts)ghost_ident;
}
