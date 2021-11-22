#include "classes.h"
#include "balls.h"

#include <iostream>
#include <math.h>

#include <cstdio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Ball::Ball( double x, double y, int id ): x(x), y(y), vx(0), vy(0), on_table(true), id(id), hit_band(false), moving(false){
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

int t = 0;
void Ball::update( Uint32 dt ){
	if( !this->on_table )
		return;
	//updating position of ball based on real time difference between frames
	this->x += this->vx * (double)dt/100;
	this->y += this->vy * (double)dt/100;
	//updating speed based on real time difference between frames
	this->vx -= 1 * sgn(this->vx) * (double)dt/100;
	this->vy -= 1 * sgn(this->vy) * (double)dt/100;
	//if the ball is slow enough we can asume it stopped
	if( sqrt( this->vx*this->vx + this->vy*this->vy ) < 0.01 ){
		this->vx = 0;
		this->vy = 0;
		this->moving = false;
	}
}

void Ball::pocket(){
	this->vx = 0;
	this->vy = 0;
	this->on_table = false;
} 

CueBall::CueBall( double x, double y ): Ball( x, y, 0 ), movable(false){}
void CueBall::pocket(){
	this->Ball::pocket();
	this->movable = true;
	this->x = 300;
	this->y = 400;
}

void CueBall::move( double x, double y ){
	this->on_table = true;
	if(!this->movable)
		return;
	//if the ball is to be placed out of the table - clip it
	if( x < XOFF + 68 + 14 ){
		x = XOFF + 68 + 14;
	}
	else if( x > XOFF + 992 - 14 ){
		x = XOFF+992 - 14;
	}
	if( y < YOFF + 68  +14 ){
		y = YOFF+68 +14;
	}else if( y > YOFF + 529 - 14 ){
		y = YOFF + 529-14;
	}
	this->x = x;
	this->y = y;
}

void CueBall::strike( double vx, double vy ){
	this->vx = vx;
	this->vy = vy;
	this->movable = false;
	this->moving = true;
}

void Ball::draw( Game *g ){
	SDL_Rect pos = { (int)this->x-25, (int)this->y-25, 50, 50 };
	SDL_RenderCopy( g->r, g->ball_textures[this->id], NULL, &pos );
}


bool Ball::check_collision( std::shared_ptr<Ball> b ){
	//don't check collisions with balls out of the table
	if( !this->on_table || !b->on_table )
		return false;
	//simple check if two circles are overlapping
	return sqrt( pow( b->x - this->x, 2 ) + pow( b->y - this->y, 2 ) ) < 28;
}

void Ball::collide( std::shared_ptr<Ball> b ){
	double dx = this->x - b->x;
	double dy = this->y - b->y;
	double dvx = this->vx - b->vx;
	double dvy = this->vy - b->vy;
	double a = (dx * dvx + dy * dvy)/(pow( dx, 2 ) + pow(dy, 2));

	//looking for point in time when balls collided
	double dVx = this->vx - b->vx;
	double dVy = this->vy - b->vy;

	double ta = dvy * dvy + dvx * dvx;
	double tb = -2* ( dx*dvx + dy * dvy );
	double tc = dx*dx+dy*dy -784;

	double delta = tb * tb - 4 * ta * tc;

	double t1 = (-tb - sqrt(delta))/(2*ta);
	double t2 = (-tb + sqrt(delta))/(2*ta);
	double t = t1<t2 ? t2:t1;

	if( a == 0 ){
		return;
	}
	//"moving back in time" to the moment of collision
	this->x -= this->vx*t;
	this->y -= this->vy*t;
	b->x -= b->vx * t;
	b->y -= b->vy*t;

	//applying velocity changes
	this->vx = this->vx - a * dx;
	this->vy = this->vy - a * dy;

	b->vx = b->vx + a*dx;
	b->vy = b->vy + a*dy;
	//going back to original time
	this->x += this->vx*t;
	this->y += this->vy*t;
	b->x += b->vx * t;
	b->y += b->vy*t;

	//if speed of balls is low enough we can set it to zero 

	if( sqrt( this->vx*this->vx + this->vy*this->vy ) < 0.01 ){
		this->vx = 0;
		this->vy = 0;
		this->moving = false;
	}
	else{
		this->moving = true;
	}
	if( sqrt( b->vx*b->vx + b->vy*b->vy ) < 0.01 ){
		b->vx = 0;
		b->vy = 0;
		b->moving = false;
	}
	else{
		b->moving = true;
	}

}

void Ball::collide( Segment *s ){
	this->hit_band = true;
	/*
	*	To get the velocity after the collision we need to invert its part parallel to the normal line of the segment
	*   to achieve this we need to subtract from initial vector doubled length of the projection (dot product) multiplied by normal vector of unit length
	*/
	double a = 2*(this->vx * s->yv - this->vy * s->xv);
	double vx = this->vx - a * s->yv;
	double vy = this->vy + a * s->xv;

	this->vx = vx;
	this->vy = vy;

}

bool Ball::check_for_pocket(){
	//coordinates of pockets with respect to top left corner of table
		double pockets[][2] = {
		{51, 51},
		{51, 546},
		{529, 46},
		{529, 551},
		{1008, 51},
		{1008, 546}
	};
	for( int i = 0; i < 6; i++ ){
		if( sqrt( pow( this->x - pockets[i][0] - XOFF, 2 ) + pow( this->y - pockets[i][1] - YOFF, 2 ) ) < ( (i == 2 || i == 3) ? 22 : 25  ) ){
			this->pocket();
			return true;
		}
	}
	return false;
}

Solid::Solid( double x, double y, int id ): Ball( x, y, id ){}
Stripe::Stripe( double x, double y, int id ): Ball( x, y, id ){}
EightBall::EightBall( double x, double y ): Ball( x, y, 8 ){}
