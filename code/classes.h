/**@file classes.h
 * 
 */

#ifndef CLASSES_H
#define CLASSES_H

#include "balls.h"

#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <typeinfo>
#include <SDL2/SDL_ttf.h>


class Game;
class Ball;

/**
 * Class used to represent boundaries of the table and check for collisions
 */
class Segment{
	
	double x;///<x coordinate of initial point 
	double y;///<y coordinate of initial point 
	
	double xv;///<x component of direction vector of the segment
	double yv;///<y component of direction vector of the segment
	
	double l;///<length of segment

public:
	/**constructor generating segment from end points
	 * @param ax x coordinate of initial point
	 * @param ay y coordinate of initial point
	 * @param bx x coordinate of terminal point
	 * @param by y coordinate of terminal point
	 */
	Segment( double ax, double ay, double bx, double by );
	/**function for checking collision with ball
	 * @param ball shared pointer to ball to check collision against
	 *@return true if collision has occured
	 */
	bool check_collision( std::shared_ptr<Ball> ball );

	friend class Ball;
};

/**
 * Class responsible for rendering text on screen
 * 
 */
class TextField{
	///pointer to SDL rendering context
	SDL_Renderer *r;
	///pointer to texture containing text
	SDL_Texture *t;
	///time left to vanish, -1 if text shouldn't be timed out
	int timeout;
	///flag set if text should be visible
	bool visible;
	///x coordinate
	int x;
	///y coordinate
	int y;

public:
	/**constructor assigning values to the text field and generating the texture from text
	 * @param r pointer to SDL rendering context
	 * @param text constant text to be rendered by this field
	 * @param font pointer to font
	 * @param x x coordinate
	 * @param y y coordinate
	 */ 
	TextField( SDL_Renderer *r, const char* text, TTF_Font *font, int x, int y );
	/**function responsible for drawing visible text field and updating time left on screen
	 * @param dt time elapsed since last call to this function
	 */
	void draw( Uint32 dt );
	///function hiding text field
	void hide();
	///function showing text field
	void show();
	/**function setting text field's timeout
	 * @param timeout time for which the text field will be visible on screen, -1 for infinite
	 */
	void setTimeout( int timeout );
	///destructor needed to destroy texture
	~TextField();
};

class Game{
	///if this flag is 0, the game stops
	bool running;
	///variable for measuring time between frames
	Uint32 lastLoop;
	///pointer to the window
	SDL_Window* window;
	///pointer to SDL rendering context
	SDL_Renderer *r;
	/**function loading textures from files
	 * @param fname name of the graphics file to be loaded as texture
	 * @return pointer to SDL texture
	 */
	SDL_Texture* loadTexture(const char * fname);

	///pointer to table texture
	SDL_Texture* table;
	///vector of pointers to ball textures
	std::vector<SDL_Texture*> ball_textures;
	///pointer to cue texture
	SDL_Texture* cue;
	///pointer to texture of powerbar background
	SDL_Texture* power_background;
	///pointer to texture of powerbar foreground
	SDL_Texture* power_foreground;


	//pool related variables

	///flag showing whose move it is
	bool player_one_turn = true;

	///vector storing pointers to all balls(ones on the table and pocketed ones)
	std::vector< std::shared_ptr<Ball> > balls;

	///current state of the game
	enum{ WAITING, MOVING_BALL, MOVING_CUE, SETTING_POWER, ROLLING, END }state;
	///variable tracking mouse movement along vertical axis to set strike power
	int last_y;
	///number of solid balls left on table
	int solids_left = 7;
	///number of striped balls left on table
	int stripes_left = 7;

	///representation of boundaries of table
	std::vector< Segment* > bands;
	///angle of cues rotation
	double alpha=0;
	///power of strike
	double power = 0;
	///flag set if white ball didn't hit any other ball in current turn
	bool first_hit = true;
	
	///pointer used for RTTI idenfification of balls belonging to player 1
	std::shared_ptr<Ball> player_one_balls;

	///flag set to true if balls have been assigned, used for faul checking and sanity checks to not try typeid on nullptr
	bool balls_assigned = false;
	///flag set if it is first shot of the game
	bool break_shot = true;
	///flag set if player pocketed at least one of his balls in current turn
	bool right_ball_pocketed = false;
	///flag set if black ball has been pocketed, pocketing it after break shot means end of the game
	bool black_out_of_table = false;
	///flag set if foul has been commited in current turn
	bool foul = false;
	///function rendering elements that cannot be timeouted
	void render();
	///function taking care of SDL events - moving mouse, keypresses, closing window e.t.c.
	void handle_events();
	///flag set if any ball has been pocketed in current turn - exception to foul when no ball hits band
	bool ball_pocketed;

	//GUI text fields

	TextField* Player1;
	TextField* Player2;
	TextField* ball_in_hand;
	TextField* failed_to_hit_ball;
	TextField* failed_to_hit_band;
	TextField* foul_message;
	TextField* Player1wins;
	TextField* Player2wins;
	TextField* pocketed_cue_ball;
	TextField* solids1;
	TextField* solids2;
	TextField* stripes1;
	///pointers to text fields that can be visible on screen
	TextField* stripes2;

public:
	///constructor taking care of initialising SDL, creating window and loading textures
	Game();
	///main loop of the game
	void run();

	///destructor takes care of proper release of SDL resources
	~Game();
	friend class Ball;
};



#endif