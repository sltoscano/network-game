
#include "body.h"

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

Body::Body(Settings* settings)
{
	m_settings = settings;

	b2Vec2 gravity;
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

	float32 x = m_settings->worldExtents.x;
	float32 y = m_settings->worldExtents.y;

	// bottom
	groundBox.SetAsEdge(b2Vec2(0,0), b2Vec2(x,0));
	m_groundBody->CreateFixture(&groundBox,0);

	// top
	groundBox.SetAsEdge(b2Vec2(0,y), b2Vec2(x,y));
	m_groundBody->CreateFixture(&groundBox,0);

	// left
	groundBox.SetAsEdge(b2Vec2(0,y), b2Vec2(0,0));
	m_groundBody->CreateFixture(&groundBox,0);

	// right
	groundBox.SetAsEdge(b2Vec2(x,y), b2Vec2(x,0));
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

void Body::Step()
{
	float32 timeStep = m_settings->hz > 0.0f ? 1.0f / m_settings->hz : float32(0.0f);

	if (m_settings->pause)
	{
		if (m_settings->singleStep)
		{
			m_settings->singleStep = 0;
		}
		else
		{
			timeStep = 0.0f;
		}

		m_debugDraw.DrawString(5, m_textLine, "****PAUSED****");
		m_textLine += 15;
	}

	uint32 flags = 0;
	flags += m_settings->drawShapes			* b2DebugDraw::e_shapeBit;
	flags += m_settings->drawJoints			* b2DebugDraw::e_jointBit;
	flags += m_settings->drawAABBs			* b2DebugDraw::e_aabbBit;
	flags += m_settings->drawPairs			* b2DebugDraw::e_pairBit;
	flags += m_settings->drawCOMs				* b2DebugDraw::e_centerOfMassBit;
	m_debugDraw.SetFlags(flags);

	m_world->SetWarmStarting(m_settings->enableWarmStarting > 0);
	m_world->SetContinuousPhysics(m_settings->enableContinuous > 0);

	m_pointCount = 0;

	m_world->Step(timeStep, m_settings->velocityIterations, m_settings->positionIterations);

	m_world->DrawDebugData();

	if (timeStep > 0.0f)
	{
		++m_stepCount;
	}

	if (m_settings->drawStats)
	{
		m_debugDraw.DrawString(5, m_textLine, "bodies/contacts/joints/proxies = %d/%d/%d",
			m_world->GetBodyCount(), m_world->GetContactCount(), m_world->GetJointCount(), m_world->GetProxyCount());
		m_textLine += 15;
	}
}
