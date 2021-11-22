/**@file balls.h
 * 
 * 
 */

#ifndef BALLS_H
#define BALLS_H

#include <SDL2/SDL.h>
#include <vector>
#include <memory>

#include "classes.h"

#define XOFF 120
#define YOFF 101
#define WIDTH 924
#define HEIGHT 461

class Game;
class Segment;

/**
 * Class representing basic billard ball
 */
class Ball{
protected:
	///Billard balls are numbered, cue ball is assumed to have id 0
	int id;

	///x coordinate
	double x;
	///y coordinate
	double y;
	///x component of velocity
	double vx;
	///y component of velocity
	double vy;
	/**this flag is true if ball is still in the game, if it's false the 
	ball isn't drawn neither checked for collisions*/
	bool on_table;
	/**this flag is set to true if ball hit the band in this turn 
	to check if the faul has been commited*/
	bool hit_band;

	///flag set to true if ball is moving - used to determine end of turn
	bool moving;
	
	///action taken after pocketing a ball, different for "normal" balls, cue ball and for eight ball
	virtual void pocket();

public:
	/** ball requires specifying position and balls number
	 *@param x x coordinate
	 *@param y y coordinate
	 *@param id number of the ball
	 */
	Ball( double x, double y, int id );
	/**function responsible for physics and checking for potting and fauls
	 *@param dt time elapsed from last update in milliseconds
	*/
	void update( Uint32 dt );
	/**function responsible for displaying the ball
	 *@param r pointer to Game object holding apropriate rendering context
	 */
	void draw( Game* r );
	/**function responsible for checking if collision with ball b occured
	 *@param b shared pointer to ball to check collisions against
	 *@return true if the collision occured
	 */
	bool check_collision( std::shared_ptr<Ball> b );
	/**function responsible for handling collision with segment
	 *@param s pointer to segment to collide with
	 */
	void collide( Segment *s );
	/**function responsible for handling collision with other ball
	 *@param b shared pointer to ball to collide with
	 */
	void collide( std::shared_ptr<Ball> b );
	/**function checking if ball has been pocketed
	 *@return function returns true if the ball has been pocketed
	 */
	bool check_for_pocket();

	friend class Segment;
	friend class Game;
};



class CueBall:public Ball{
	///flag set if player has the ball in hand
	bool movable;
	///cue ball after pocketing returns to the table
	void pocket();

public:
	/**cue ball always has id 0, so it doesn't need to be passed to constructor
	 *@param x x coordinate
	 *@param y y coordinate
	 */
	CueBall( double x, double y );
	/**function responsible for moving the ball if player has it in hand
	 * @param x destination x coordinate
	 * @param y destination y corrdinate
	 */
	void move( double x, double y );
	/**function initiating movement of white ball
	 * @param vx horizontal element of velocity
	 * @param vy vertical element of velocity
	 */
	void strike( double vx, double vy );
	friend class Game;
};


///separate class for RTTI and to get constant id
class EightBall: public Ball{
public:
	/**cue ball always has id 8, so it doesn't need to be passed to constructor
	 *@param x x coordinate
	 *@param y y coordinate
	 */
	EightBall( double x, double y );
};
///separate class for RTTI
class Solid: public Ball{
public:
	/**
	 *@param x x coordinate
	 *@param y y coordinate
	 */
	Solid( double x, double y, int id );
};
///separate class for RTTI
class Stripe: public Ball{
public:
	/**
	 *@param x x coordinate
	 *@param y y coordinate
	 */
	Stripe( double x, double y, int id );
};


#endif