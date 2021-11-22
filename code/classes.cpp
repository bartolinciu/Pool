#include <iostream>
#include <math.h>

#include <cstdio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <windows.h>

#include "classes.h"

//y = 169
//x = 188

#define XOFF 120
#define YOFF 101

#define PI 3.14159265




Game::Game():  state(WAITING)
{
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ){
		std::cout<<"Initialisation failed! Error:"<< SDL_GetError()<<std::endl;
	}

	this->window = SDL_CreateWindow( "Pool 2D", SDL_WINDOWPOS_CENTERED, 
									 SDL_WINDOWPOS_CENTERED, 1300, 800, 
									 SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );


	if( this->window == NULL ){
		std::cout<<"Couldn't create window! Error: "<<SDL_GetError()<<std::endl;
	}
	
	this->r = SDL_CreateRenderer( this->window, -1, SDL_RENDERER_ACCELERATED );
			if(this->r == NULL){
		std::cout<< "Renderer couldn't be created! SDL Error: "<< SDL_GetError()<<std::endl;
	}


	SDL_SetRenderDrawBlendMode( this->r, SDL_BLENDMODE_BLEND );
	SDL_SetRenderDrawColor( this->r, 255, 255, 255, 255 );


	if( !( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) ){
		std::cout<<"SDL_image could not initialize! SDL_image Error: "<< IMG_GetError()<<std::endl;
	}

	this->table = this->loadTexture( "gfx/table.png" );
	this->cue = this->loadTexture( "gfx/cue.png" );
	this->power_background = this->loadTexture( "gfx/powerbar_background.png" );
	this->power_foreground = this->loadTexture( "gfx/powerbar_foreground.png" );
	for( int i = 0; i < 16; i++ ){
		std::string fname = std::string("gfx/") + std::to_string(i) + std::string(".png");
		this->ball_textures.push_back( this->loadTexture(fname.c_str()) );
	}

	double xoff = 800;
	double yoff = 400;

	double ball_coords[][2] = {
		{ 0, 0 },
		{ 104, -56 },
		{ 26, -14 },
		{ 52, 28 },
		{ 52, -28 },
		{ 78, 14 },
		{ 78, 42 },
		{ 52, 0 },
		{ 78, -14 },
		{ 78, -42 },
		{ 104, 0 },
		{ 104, 28 },
		{ 104, 56 },
		{ 104, -28 },
		
		{ 26, 14 }

	};

	this->balls.push_back( std::shared_ptr<Ball>(new CueBall(300, 400)));

	for( int i = 0; i < 7; i++ ){
		this->balls.push_back( std::shared_ptr<Ball>( new Solid( xoff+ball_coords[i][0], yoff + ball_coords[i][1], i+1 ) ) );
	}
	this->balls.push_back( std::shared_ptr<Ball>( new EightBall( 852, 400 ) ) );
	for( int i = 8; i < 15; i++ ){
		this->balls.push_back( std::shared_ptr<Ball>( new Stripe( xoff+ball_coords[i][0], yoff + ball_coords[i][1], i+1 ) ) );
	}


	double points[][4]={
		{50, 79, 68, 96},
		{68, 96, 68, 501},
		{68, 501, 50, 518},

		{78, 548, 96, 529},
		{96, 529, 504, 529},
		{504, 529, 508, 548},

		{552, 548, 556, 529},
		{556, 529, 963, 529},
		{963, 529, 981, 548},

		{1010, 518, 992, 501},
		{992, 501, 992, 96},
		{992, 96, 1010, 79},

		{981, 50, 963, 68},
		{963, 68, 556, 68},
		{556, 68, 552, 50},

		{508, 50, 504, 68},
		{504, 68, 96, 68},
		{96, 68, 78, 50}
	};

	for( int i=0; i < sizeof(points)/(4*sizeof(double)); i++ ){
		this->bands.push_back( new Segment( XOFF + points[i][0], YOFF + points[i][1], XOFF + points[i][2], YOFF + points[i][3] ) );
	}


	if(TTF_Init()==-1) {
	    std::cout<<"SDL_TTF could not initialize! SDL_TTF Error: "<< TTF_GetError()<<std::endl;
	    return;
	}


	TTF_Font * font = NULL;

	if(! (font = TTF_OpenFont("Roboto-Regular.ttf", 32) ) ){
	   	std::cout<<"TTF_OpenFont: "<< TTF_GetError()<<std::endl;
	   	return;
	}

	this->Player1 = new TextField( this->r, "Player 1", font, XOFF, 20 );
	this->Player1->show();
	this->Player2 = new TextField( this->r, "Player 2", font, XOFF + 900, 20 );
	this->ball_in_hand = new TextField( this->r, "Player has the ball in hand",font,  XOFF + 350, YOFF + 600 );
	this->ball_in_hand->show();
	this->foul_message = new TextField( this->r, "foul", font, XOFF + 500, 368 );
	this->failed_to_hit_ball = new TextField( this->r, "failed to hit correct ball first", font,  XOFF + 300, 400 );
	this->failed_to_hit_band = new TextField( this->r, "no ball hit bands", font, XOFF+400, 400 );
	this->Player1wins = new TextField( this->r, "Player 1 wins", font, XOFF + 350, 20 );
	this->Player2wins = new TextField( this->r, "Player 2 wins", font, XOFF + 350, 20 );
	this->pocketed_cue_ball = new TextField( this->r, "pocketed cue ball", font, XOFF + 430, 432 );
	this->solids1 = new TextField(this->r, "Solids", font, XOFF, 52);
	this->solids2 = new TextField( this->r, "Solids", font, XOFF+900, 52 );
	this->stripes1 = new TextField(this->r, "Stripes", font, XOFF, 52);
	this->stripes2 = new TextField( this->r, "Stripes", font, XOFF+900, 52 );
}


void Game::run(){
	this->running = true;
	this->lastLoop = SDL_GetTicks();

	while( this->running ){

		this->handle_events();
		
		Uint32 ticks = SDL_GetTicks();
		Uint32 dt = ticks - this->lastLoop;
		this->lastLoop = ticks;

		SDL_RenderClear( this->r );
		this->render();

		this->Player1->draw( dt );
		this->Player2->draw( dt );
		this->foul_message->draw(dt);
		this->failed_to_hit_ball->draw(dt);
		this->failed_to_hit_band->draw(dt);
		this->Player1wins->draw(dt);
		this->Player2wins->draw(dt);
		this->pocketed_cue_ball->draw(dt);
		this->solids1->draw(dt); 
		this->solids2->draw(dt);
		this->stripes1->draw(dt);
		this->stripes2->draw(dt); 
		if( std::dynamic_pointer_cast<CueBall>(this->balls[0])->movable ){
			this->ball_in_hand->draw(dt);
		}

		SDL_RenderPresent(this->r);

		if(this->state == MOVING_BALL || this->state == END)
			continue;



		bool balls_moving = false;
		if(this->state == ROLLING)
			for( const auto& ball: this->balls ){
				if( ball->on_table ){
					ball->update( dt );
					if( ball->moving )
						balls_moving = true;

					if(ball->check_for_pocket() ){
						this->ball_pocketed = true;

						if( typeid(*ball) == typeid(EightBall) ){
							if( this->break_shot ){
								ball->x = 852;
								ball->y = 400;
							}
							else{
								black_out_of_table = true;
							}
						}else if( typeid( *ball ) == typeid(CueBall) ){
							foul = true;
							this->foul_message->show();
							this->foul_message->setTimeout(3000);
							this->pocketed_cue_ball->show();
							this->pocketed_cue_ball->setTimeout(3000);
						}else{ 
							if( typeid(*ball) == typeid(Solid) )
								solids_left--;
							else{
								stripes_left--;
							}

							if( !this->balls_assigned ){
								right_ball_pocketed = true;
								if( !this->break_shot ){
									if( player_one_turn )
										this->player_one_balls = ball;
									else{
										this->player_one_balls =  typeid(*ball) == typeid(Solid) ? this->balls[9] : this->balls[1];
									}
									this->balls_assigned = true;
									if( (typeid( *this->player_one_balls ) == typeid( Solid ) )){
										this->solids1->show();
										this->stripes2->show();
									}
									else{
										this->solids2->show();
										this->stripes1->show();
									}

								}
							}else{
								if( ( this->player_one_turn && (typeid( *ball ) == typeid(*this->player_one_balls) ) || !this->player_one_turn && typeid( *ball ) != typeid( *this->player_one_balls )  ) )
									right_ball_pocketed = true;
							}
						}

					}
				}
			}

		
		
		
		for( int i = 0; i < this->balls.size(); i++ ){
			for( auto s: this->bands ){
				if( s->check_collision( this->balls[i] ) ){
					this->balls[i]->collide( s );
				}
			}
		}
		
		std::shared_ptr<Ball> now_played;
		if(balls_assigned)
			now_played = player_one_turn ? player_one_balls : ( player_one_balls->id < 8 ? this->balls[9] : this->balls[1] );
		else
			now_played = nullptr;
		for( int i = 0; i < this->balls.size()-1; i++ ){
			for( int j = i+1; j<this->balls.size(); j++ ){
				if( this->balls[i]->check_collision( this->balls[j] ) ){
					
					
					if( first_hit && i == 0 && (  (j == 8 && ( !balls_assigned || balls_assigned && (typeid( *now_played ) == typeid( Solid ) ? solids_left : stripes_left) != 0 )) || (j!=8 && balls_assigned && (typeid( *this->balls[j] ) != typeid(*now_played) )  ) ) ){
						this->foul_message->show();
						this->foul_message->setTimeout(3000);
						this->failed_to_hit_ball->show();
						this->failed_to_hit_ball->setTimeout(3000);
						foul = true;
						std::dynamic_pointer_cast<CueBall>(this->balls[0])->movable = true;
					}

					if( first_hit && i == 0 ){
						first_hit = false;
					}
					this->balls[i]->collide(this->balls[j]);
				}
			}
		}	

		if( this->state != ROLLING || balls_moving )
			continue;

		if( first_hit ){
			this->foul_message->show();
			this->foul_message->setTimeout(3000);
			this->failed_to_hit_ball->show();
			this->failed_to_hit_ball->setTimeout(3000);
			foul = true;
			std::dynamic_pointer_cast<CueBall>(this->balls[0])->movable = true;
		}

		bool hit_band = false;
		for( const auto& ball:this->balls ){
			if( ball->hit_band )
				hit_band = true;
		}

		if(  !hit_band && !ball_pocketed && !foul ){
			//std::cout<<"foul - no ball hit bands"<<std::endl;
			this->foul_message->show();
			this->foul_message->setTimeout(3000);
			this->failed_to_hit_band->show();
			this->failed_to_hit_band->setTimeout(3000);
			std::dynamic_pointer_cast<CueBall>(this->balls[0])->movable = true;
			foul = true;
		}



		this->state = WAITING;
		this->balls[0]->on_table = true;
		break_shot = false;
		int players_balls_left = 7;
		if( balls_assigned )
			players_balls_left = typeid(*now_played) == typeid(Solid) ? solids_left : stripes_left;
		if( black_out_of_table ){
			this->Player1->hide();
			this->Player2->hide();
			if( player_one_turn ^ (foul|| players_balls_left !=0 ) ){
				this->Player1wins->show();

			}
			else{
				this->Player2wins->show();

			}

			this->state = END;
			continue;
		}



		if( foul || !right_ball_pocketed ){
			player_one_turn = !player_one_turn;
			if( player_one_turn ){
				this->Player1->show();
				this->Player2->hide();
			}
			else{
				this->Player1->hide();
				this->Player2->show();
			}

		}
	}
}



SDL_Texture* Game::loadTexture(const char* fname){
	SDL_Surface *image;
	image = IMG_Load(fname);
	if( !image ){
		std::cout<< "Failed to load image \""<<fname<<"\"! Error: "<< IMG_GetError()<<std::endl;
		SDL_FreeSurface(image);
		return NULL;
	}

	SDL_Texture* newTexture = SDL_CreateTextureFromSurface( this->r, image );
	if( !newTexture ){
		std::cout<<"Unable to create texture from "<<fname<<"! SDL Error: "<< SDL_GetError()<<std::endl;
		SDL_FreeSurface( image );
		return NULL;
	}

	SDL_FreeSurface(image);
	return newTexture;
}

Game::~Game(){
	SDL_DestroyTexture( this->cue );
	SDL_DestroyTexture( this->table );
	SDL_DestroyTexture( this->power_foreground );
	SDL_DestroyTexture( this->power_background );
	for( const auto& texture:this->ball_textures ){
		SDL_DestroyTexture(texture);
	}

	delete this->Player1;
	delete this->Player2;
	delete this->ball_in_hand;
	delete this->failed_to_hit_ball;
	delete this->foul_message;
	delete this->failed_to_hit_band;
	delete this->Player1wins;
	delete this->Player2wins;
	delete this->solids1;
	delete this->solids2;
	delete this->stripes1;
	delete this->stripes2;

	SDL_DestroyWindow( this->window );
	SDL_DestroyRenderer( this->r );
	SDL_Quit();
}

Segment::Segment( double ax, double ay, double bx, double by ):  x(ax), y(ay), l(this->l = sqrt( pow( (bx-ax), 2 ) + pow( (by-ay), 2 ) )) {
	this->xv = (bx-ax)/this->l;
	this->yv = (by-ay)/this->l;
}

bool Segment::check_collision( std::shared_ptr<Ball> ball ){
	double rx = ball->x - this->x;
	double ry = ball->y - this->y;
	double b = rx * this->yv - ry * this->xv;

	double a;
	if( this->xv != 0 ){
		a = (rx - b * this->yv)/this->xv;
	}else{
		a = (ry + b * this->xv)/this->yv;
	}
	if(b < 14 && a < this->l && a > 0){
		double db = 14-b;
		ball->x += this->yv * db;
		ball->y -= this->xv * db;
		return true;
	}

	return false;
}

void Game::render(){
	

	SDL_Rect pos = {XOFF, YOFF, 1060, 597 };
	SDL_RenderCopy( this->r, this->table, NULL, &pos );
	for( const auto& ball:this->balls ){
		if(ball->on_table)
			ball->draw(this);
	}


	if(this->state != ROLLING && this->state!= END){

		pos.x = XOFF + 1070;
		pos.y = YOFF + 104;
		pos.w = 50;
		pos.h = 371;

		SDL_RenderCopy( this->r, this->power_background, NULL, &pos );
		SDL_Rect src = {0, 0, 50, 349 - (int)(power)};
		pos.x+=11;
		pos.y+=11 + (int)(power);
		pos.w-=22;
		pos.h-=22 + (int)(power);
		SDL_RenderCopy( this->r, this->power_foreground, &src, &pos );


		pos.x = this->balls[0]->x - 17 - (int)(cos(this->alpha/180 * PI)*204);
		pos.y = this->balls[0]->y - 184 - (int)(sin(this->alpha/180 * PI)*204);;
		pos.w = 34;
		pos.h = 368;
		SDL_Point p = { 17, 184 };
		SDL_RenderCopyEx( this->r, this->cue, NULL, &pos, this->alpha + 90, &p, SDL_FLIP_NONE );
	}
	
}

void Game::handle_events(){
	SDL_Event e;
	while( SDL_PollEvent( &e ) != 0 ){
		switch( e.type ){
			case SDL_QUIT:
				this->running = false;
				break;

			case SDL_KEYDOWN:
 
				if(	this->state == WAITING )
					switch( e.key.keysym.scancode ){
						case SDL_SCANCODE_SPACE:{
							double alpha_rad = this->alpha/180 * PI;
							std::dynamic_pointer_cast<CueBall>(this->balls[0])->strike(cos( alpha_rad )*this->power/2 , sin( alpha_rad )*this->power/2 );
							this->power = 0;
							this->state = ROLLING;
							this->first_hit = true;
							this->right_ball_pocketed = false;
							this->ball_pocketed = false;
							this->foul = false;
							for( const auto& ball:this->balls ){
								ball->hit_band = false;
							}
							}
							break;
						case SDL_SCANCODE_LEFT:
							this->alpha -= 2;
							break;
						case SDL_SCANCODE_RIGHT:
							this->alpha += 2;
							break;
						case SDL_SCANCODE_DOWN:
							this->power += 10;
							if( power > 329 )
								power = 329;
							break;
						case SDL_SCANCODE_UP:
							this->power -= 10;
							if( power < 0 )
								power = 0;
							break;
					}
					

					break;
			case SDL_MOUSEBUTTONDOWN:
					if( e.button.x > XOFF+1081 && e.button.x < XOFF + 1109 && e.button.y > YOFF + 104 + power/10 && e.button.y < YOFF+104+349  ){
						this->state = SETTING_POWER;
						this->last_y = e.button.y;
							
					}else{
						double x = this->balls[0]->x - e.button.x;
						double y = this->balls[0]->y - e.button.y;
						double l = sqrt( x*x + y*y );
						if( l < 14 ){
							this->state = MOVING_BALL;
						}
						else{
							x/=l;
							double alpha = (acos(x)/PI * 180) * ( y>0 ? 1: -1  );
							if(  abs( this->alpha - alpha ) < 3 ){
								this->state =  MOVING_CUE;
							}
						}
					}
					break;

			case SDL_MOUSEMOTION:
				if( this->state == MOVING_BALL ){
					double x = this->balls[0]->x;
					double y = this->balls[0]->y;
					std::dynamic_pointer_cast<CueBall>(this->balls[0])->move( e.motion.x, e.motion.y );
					for( int i = 1; i < 16; i++ ){
						if( this->balls[0]->check_collision( this->balls[i] ) ){
							this->balls[0]->x = x;
							this->balls[0]->y = y;
							break;
						}
					}

					SetCursor( LoadCursorA(NULL, IDC_HAND) );
				}else if( this->state == SETTING_POWER ){
					int dy = e.motion.y - this->last_y;
					this->power += dy;
					this->last_y = e.motion.y;
					if( this->power >= 329 )
						this->power =329;
					else if( this->power < 0 )
						this->power = 0; 
					SetCursor( LoadCursorA(NULL, IDC_HAND) );
				}else if(e.button.x > XOFF+1081 && e.button.x < XOFF + 1109 && e.button.y > YOFF + 104 + power && e.button.y < YOFF+104+349){
					SetCursor( LoadCursorA(NULL, IDC_HAND) );
				}else{ 
					double x = this->balls[0]->x - e.motion.x;
					double y = this->balls[0]->y - e.motion.y;
					double l = sqrt( x*x + y*y );
					x/=l;

					double alpha = (acos(x)/PI * 180) * ( y>0 ? 1: -1  );

					if( this->state == MOVING_CUE ){

						SetCursor( LoadCursorA(NULL, IDC_HAND) );
							
						this->alpha = alpha;

					}else{
						if( abs( this->alpha - alpha ) < 3 || (l < 14 && std::dynamic_pointer_cast<CueBall>(this->balls[0])->movable ) ){
							SetCursor( LoadCursorA(NULL, IDC_HAND) );
						}

					}
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if(this->state != ROLLING && this->state != END)
					this->state = WAITING;
				break;


			default:
				break;
			}
		}
}

SDL_Texture* renderTextToTexture( SDL_Renderer* r, const char* text, TTF_Font* font ){

    SDL_Surface *text_surface;
    SDL_Color color = {0,0,0, 255};
    text_surface = TTF_RenderUTF8_Blended(font, text, color );
    if( !text_surface ){
    	std::cout<<"Unable to render text! SDL Error: "<< SDL_GetError()<<std::endl;
    	return NULL;
    }
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface( r, {text_surface} );
	if( newTexture == NULL ){
		std::cout<< "Unable to create texture from surface! SDL Error: "<< SDL_GetError()<<std::endl;
		return NULL;
	}
	return newTexture;
}

TextField::TextField( SDL_Renderer *r, const char* text, TTF_Font *font, int x, int y ):r(r), x(x), y(y), visible(false), timeout(-1){
	this->t = renderTextToTexture( r, text, font );
}


void TextField::show(){
	this->visible = true;
}
void TextField::hide(){
	this->visible = false;
}
void TextField::setTimeout( int timeout ){
	this->timeout = timeout;
}
void TextField::draw( Uint32 dt ){
	if( !this->visible )
		return;
	if( this->timeout != -1 ){
		this->timeout -= dt;
		if( this->timeout < 0 ){
			this->timeout = -1;
			this->visible = false;
			return;
		}
	}

	SDL_Rect pos;
	pos.x = this->x;
	pos.y = this->y;


	SDL_QueryTexture( this->t, NULL, NULL, &pos.w, &pos.h );

	SDL_RenderCopy( this->r, this->t, NULL, &pos );
}

TextField::~TextField(){
	SDL_DestroyTexture(this->t);
}