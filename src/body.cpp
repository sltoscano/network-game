
#include "body.h"

#include "freeglut/GL/freeglut_std.h"
#include <cstdio>


void DestructionListener::SayGoodbye(b2Joint* joint)
{
	if (body->m_mouseJoint == joint)
	{
		body->m_mouseJoint = NULL;
	}
	else
	{
		body->JointDestroyed(joint);
	}
}

b2Vec2 ConvertScreenToWorld(int32 x, int32 y)
{
	int tw = glutGet(GLUT_WINDOW_WIDTH);
	int th = glutGet(GLUT_WINDOW_HEIGHT);

	float32 u = x / float32(tw);
	float32 v = (th - y) / float32(th);

	float32 ratio = float32(tw) / float32(th);
	b2Vec2 extents(ratio * 25.0f, 25.0f);

	float32 viewZoom = 1.0f;

	extents *= viewZoom;

	b2Vec2 viewCenter(0.0f, 0.0f);

	b2Vec2 lower = viewCenter - extents;
	b2Vec2 upper = viewCenter + extents;

	b2Vec2 p;
	p.x = (1.0f - u) * lower.x + u * upper.x;
	p.y = (1.0f - v) * lower.y + v * upper.y;
	return p;
}

Body::Body()
{
	b2Vec2 gravity;
	//gravity.Set(0.0f, -10.0f);
	gravity.Set(0.0f, 0.0f);
	bool doSleep = true;
	m_world = new b2World(gravity, doSleep);
	m_textLine = 30;
	m_mouseJoint = NULL;
	m_pointCount = 0;

	m_destructionListener.body = this;
	m_world->SetDestructionListener(&m_destructionListener);
	m_world->SetContactListener(this);
	m_world->SetDebugDraw(&m_debugDraw);
	
	m_stepCount = 0;

	b2BodyDef bodyDef;
	bodyDef.position.Set(0, 0); // bottom-left corner
	m_groundBody = m_world->CreateBody(&bodyDef);
	
	// Define the ground box shape.
	b2PolygonShape groundBox;

	b2Vec2 vec = ConvertScreenToWorld(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	float32 w = b2Abs(vec.x);
	float32 h = b2Abs(vec.y);

	// bottom
	groundBox.SetAsEdge(b2Vec2(0,0), b2Vec2(w,0));
	m_groundBody->CreateFixture(&groundBox,0);

	// top
	groundBox.SetAsEdge(b2Vec2(0,h), b2Vec2(w,h));
	m_groundBody->CreateFixture(&groundBox,0);

	// left
	groundBox.SetAsEdge(b2Vec2(0,h), b2Vec2(0,0));
	m_groundBody->CreateFixture(&groundBox,0);

	// right
	groundBox.SetAsEdge(b2Vec2(w,h), b2Vec2(w,0));
	m_groundBody->CreateFixture(&groundBox,0);
}

Body::~Body()
{
	/// By deleting the world, we delete the bomb, mouse joint, etc.
	delete m_world;
	m_world = NULL;
}

void Body::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	const b2Manifold* manifold = contact->GetManifold();

	if (manifold->pointCount == 0)
	{
		return;
	}

	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
	b2GetPointStates(state1, state2, oldManifold, manifold);

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);

	for (int32 i = 0; i < manifold->pointCount && m_pointCount < k_maxContactPoints; ++i)
	{
		ContactPoint* cp = m_points + m_pointCount;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = worldManifold.points[i];
		cp->normal = worldManifold.normal;
		cp->state = state2[i];
		++m_pointCount;
	}
}

void Body::DrawTitle(int x, int y, const char *string)
{
	m_debugDraw.DrawString(x, y, string);
}

class QueryCallback : public b2QueryCallback
{
public:
	QueryCallback(const b2Vec2& point)
	{
		m_point = point;
		m_fixture = NULL;
	}

	bool ReportFixture(b2Fixture* fixture)
	{
		b2Body* body = fixture->GetBody();
		if (body->GetType() == b2_dynamicBody)
		{
			bool inside = fixture->TestPoint(m_point);
			if (inside)
			{
				m_fixture = fixture;

				// We are done, terminate the query.
				return false;
			}
		}

		// Continue the query.
		return true;
	}

	b2Vec2 m_point;
	b2Fixture* m_fixture;
};

void Body::Step(Settings* settings)
{
	float32 timeStep = settings->hz > 0.0f ? 1.0f / settings->hz : float32(0.0f);

	if (settings->pause)
	{
		if (settings->singleStep)
		{
			settings->singleStep = 0;
		}
		else
		{
			timeStep = 0.0f;
		}

		m_debugDraw.DrawString(5, m_textLine, "****PAUSED****");
		m_textLine += 15;
	}

	uint32 flags = 0;
	flags += settings->drawShapes			* b2DebugDraw::e_shapeBit;
	flags += settings->drawJoints			* b2DebugDraw::e_jointBit;
	flags += settings->drawAABBs			* b2DebugDraw::e_aabbBit;
	flags += settings->drawPairs			* b2DebugDraw::e_pairBit;
	flags += settings->drawCOMs				* b2DebugDraw::e_centerOfMassBit;
	m_debugDraw.SetFlags(flags);

	m_world->SetWarmStarting(settings->enableWarmStarting > 0);
	m_world->SetContinuousPhysics(settings->enableContinuous > 0);

	m_pointCount = 0;

	m_world->Step(timeStep, settings->velocityIterations, settings->positionIterations);

	m_world->DrawDebugData();

	if (timeStep > 0.0f)
	{
		++m_stepCount;
	}

	if (settings->drawStats)
	{
		m_debugDraw.DrawString(5, m_textLine, "bodies/contacts/joints/proxies = %d/%d/%d",
			m_world->GetBodyCount(), m_world->GetContactCount(), m_world->GetJointCount(), m_world->GetProxyCount());
		m_textLine += 15;
	}
}
