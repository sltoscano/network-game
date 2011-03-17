
#ifndef WORLD_H
#define WORLD_H

#include "common.h"
#include "render.h"
#include <Box2D/Box2D.h>

#include <cstdlib>


class Body;
struct Settings;

typedef Body* EntityCreateFcn();

#define	RAND_LIMIT	32767

//Pixel to metres ratio. Box2D uses metres as the unit for measurement.
//This ratio defines how many pixels correspond to 1 Box2D "metre"
//Box2D is optimized for objects of 1x1 metre therefore it makes sense
//to define the ratio so that your most common object type is 1x1 metre.
#define PTM_RATIO 1


/// Random number in range [-1,1]
inline float32 RandomFloat()
{
	float32 r = (float32)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = 2.0f * r - 1.0f;
	return r;
}

/// Random floating point number in range [lo, hi]
inline float32 RandomFloat(float32 lo, float32 hi)
{
	float32 r = (float32)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = (hi - lo) * r + lo;
	return r;
}

// Settings for a world body
struct Settings
{
	Settings() :
		hz(60.0f),
		velocityIterations(8),
		positionIterations(3),
		drawStats(1),
		drawShapes(1),
		drawJoints(1),
		drawAABBs(0),
		drawPairs(0),
		drawContactPoints(0),
		drawContactNormals(0),
		drawContactForces(0),
		drawFrictionForces(0),
		drawCOMs(0),
		enableWarmStarting(1),
		enableContinuous(1),
		pause(0),
		singleStep(0),
		windowWidth(0),
		windowHeight(0)
		{}

	float32 hz;
	int32 velocityIterations;
	int32 positionIterations;
	int32 drawShapes;
	int32 drawJoints;
	int32 drawAABBs;
	int32 drawPairs;
	int32 drawContactPoints;
	int32 drawContactNormals;
	int32 drawContactForces;
	int32 drawFrictionForces;
	int32 drawCOMs;
	int32 drawStats;
	int32 enableWarmStarting;
	int32 enableContinuous;
	int32 pause;
	int32 singleStep;
	int32 windowWidth;
	int32 windowHeight;
};

struct Entity
{
	const char *name;
	EntityCreateFcn *createFcn;
};

extern Entity g_entities[];

/// This is called when a joint in the world is implicitly destroyed
/// because an attached body is destroyed. This gives us a chance to
/// nullify the mouse joint.
class DestructionListener : public b2DestructionListener
{
public:
	void SayGoodbye(b2Fixture* fixture) { REF(fixture); }
	void SayGoodbye(b2Joint* joint);

	Body* body;
};

const int32 k_maxContactPoints = 2048;

struct ContactPoint
{
	b2Fixture* fixtureA;
	b2Fixture* fixtureB;
	b2Vec2 normal;
	b2Vec2 position;
	b2PointState state;
};

class Body : public b2ContactListener
{
public:
	Body();
	virtual ~Body();

	void SetTextLine(int32 line) { m_textLine = line; }
	void DrawTitle(int x, int y, const char *string);
	virtual void Step(Settings* settings);
	virtual void Keyboard(unsigned char key) { REF(key); }
	//void ShiftMouseDown(const b2Vec2& p);
	//virtual void MouseDown(const b2Vec2& p);
	//virtual void MouseUp(const b2Vec2& p);
	//void MouseMove(const b2Vec2& p);

	/// Let derived tests know that a joint was destroyed.
	virtual void JointDestroyed(b2Joint* joint) { REF(joint); }

	/// Callbacks for derived classes.
	virtual void BeginContact(b2Contact* contact) { REF(contact); }
	virtual void EndContact(b2Contact* contact) { REF(contact); }
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	virtual void PostSolve(const b2Contact* contact, const b2ContactImpulse* impulse)
	{
		REF(contact);
		REF(impulse);
	}

protected:
	friend class DestructionListener;
	friend class BoundaryListener;
	friend class ContactListener;

	b2Body* m_groundBody;
	b2AABB m_worldAABB;
	ContactPoint m_points[k_maxContactPoints];
	int32 m_pointCount;
	DestructionListener m_destructionListener;
	DebugDraw m_debugDraw;
	int32 m_textLine;
	b2World* m_world;
	b2MouseJoint* m_mouseJoint;
	b2Vec2 m_mouseWorld;
	int32 m_stepCount;
};

#endif
