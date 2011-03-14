
#ifndef SHIP_H
#define SHIP_H

#include "body.h"

#include <freeglut/GL/freeglut.h>


const int32 k_maxBodies = 256;

/// This callback is called by b2World::QueryAABB. We find all the fixtures
/// that overlap an AABB. Of those, we use b2TestOverlap to determine which fixtures
/// overlap a circle. Up to 4 overlapped fixtures will be highlighted with a yellow border.
class PolyShapesCallback : public b2QueryCallback
{
public:
	
	enum
	{
		e_maxCount = 4
	};

	PolyShapesCallback()
	{
		m_count = 0;
	}

	void DrawFixture(b2Fixture* fixture)
	{
		b2Color color(0.95f, 0.95f, 0.6f);
		const b2Transform& xf = fixture->GetBody()->GetTransform();

		switch (fixture->GetType())
		{
		case b2Shape::e_circle:
			{
				b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();

				b2Vec2 center = b2Mul(xf, circle->m_p);
				float32 radius = circle->m_radius;

				m_debugDraw->DrawCircle(center, radius, color);
			}
			break;

		case b2Shape::e_polygon:
			{
				b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
				int32 vertexCount = poly->m_vertexCount;
				b2Assert(vertexCount <= b2_maxPolygonVertices);
				b2Vec2 vertices[b2_maxPolygonVertices];

				for (int32 i = 0; i < vertexCount; ++i)
				{
					vertices[i] = b2Mul(xf, poly->m_vertices[i]);
				}

				m_debugDraw->DrawPolygon(vertices, vertexCount, color);
			}
			break;
		}
	}

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	bool ReportFixture(b2Fixture* fixture)
	{
		if (m_count == e_maxCount)
		{
			return false;
		}

		b2Body* body = fixture->GetBody();
		b2Shape* shape = fixture->GetShape();

		bool overlap = b2TestOverlap(shape, &m_circle, body->GetTransform(), m_transform);

		if (overlap)
		{
			DrawFixture(fixture);
			++m_count;
		}

		return true;
	}

	b2CircleShape m_circle;
	b2Transform m_transform;
	b2DebugDraw* m_debugDraw;
	int32 m_count;
};

class Ship : public Body
{
public:
	Ship()
	{
		// Shape of ship
		{
			b2Vec2 vertices[3];
			/*
			vertices[0].Set(-0.75f, -0.5f);
			vertices[1].Set(1.0f, 0.0f);
			vertices[2].Set(-0.75f, 0.5f);
			*/
			/*
			//world center = 0,0
			vertices[0].Set(-0.3125f, -0.4f);
			vertices[1].Set(0.625f, 0.0f);
			vertices[2].Set(-0.3125f, 0.4f);
			*/
			vertices[0].Set(-0.75f, -0.4f);
			vertices[1].Set(0.5f, 0.0f);
			vertices[2].Set(-0.75f, 0.4f);
			
			m_polygons[0].Set(vertices, 3);
		}

		m_thrust = 0;
		m_bodyIndex = 0;
		//m_lastSpeedChange = 0;
		//m_lastDecelerate = 0;
		memset(m_bodies, 0, sizeof(m_bodies));
	}

	void Create(int32 index)
	{
		if (m_bodies[m_bodyIndex] != NULL)
		{
			m_world->DestroyBody(m_bodies[m_bodyIndex]);
			m_bodies[m_bodyIndex] = NULL;
		}

		b2BodyDef bd;
		bd.type = b2_dynamicBody;

		//float32 x = RandomFloat(-2.0f, 2.0f);
		bd.position.Set(0.0f, 0.0f);
		bd.angle = 0;

		m_bodies[m_bodyIndex] = m_world->CreateBody(&bd);

		if (index == 0)
		{
			b2FixtureDef fd;
			fd.shape = m_polygons + index;
			fd.density = 1.0f;
			fd.friction = 0.3f;
			m_bodies[m_bodyIndex]->CreateFixture(&fd);
		}

		m_bodyIndex = (m_bodyIndex + 1) % k_maxBodies;
	}

	void DestroyBody()
	{
		for (int32 i = 0; i < k_maxBodies; ++i)
		{
			if (m_bodies[i] != NULL)
			{
				m_world->DestroyBody(m_bodies[i]);
				m_bodies[i] = NULL;
				return;
			}
		}
	}

	b2Vec2 rad2vec(float r, float m = 1) {
		return b2Vec2(cosf(r)*m,sinf(r)*m);
	}

	void Keyboard(unsigned char key)
	{
		switch (key)
		{
		case '1':
			Create(0);
			break;

		case GLUT_KEY_UP:
			if (m_bodies[0])
			{
				m_thrust += 0.5f;
				m_thrust = b2Clamp(m_thrust, 0.0f, 50.0f);
				b2Vec2 vec(rad2vec(m_bodies[0]->GetAngle(), m_thrust));
				m_bodies[0]->SetLinearVelocity(vec);

				m_bodies[0]->SetAngularVelocity(0.0f);

				//m_lastSpeedChange = glutGet(GLUT_ELAPSED_TIME);
			}
			break;

		case GLUT_KEY_DOWN:
			if (m_bodies[0])
			{
				m_thrust -= 0.5f;
				m_thrust = b2Clamp(m_thrust, 0.0f, 50.0f);
				b2Vec2 vec(rad2vec(m_bodies[0]->GetAngle(), m_thrust));
				m_bodies[0]->SetLinearVelocity(vec);

				m_bodies[0]->SetAngularVelocity(0.0f);

				//m_lastSpeedChange = glutGet(GLUT_ELAPSED_TIME);
			}
			break;
		case GLUT_KEY_LEFT:
			if (m_bodies[0])
			{
				/*
				m_bodies[0]->SetTransform(m_bodies[0]->GetPosition(), 
					m_bodies[0]->GetAngle() + 0.1f);
				m_bodies[0]->SetAngularVelocity(0.0f);
				*/
				m_bodies[0]->SetAngularVelocity(1.5f);
			}
			break;
		case GLUT_KEY_RIGHT:
			if (m_bodies[0])
			{
				/*
				m_bodies[0]->SetTransform(m_bodies[0]->GetPosition(), 
					m_bodies[0]->GetAngle() - 0.1f);
				m_bodies[0]->SetAngularVelocity(0.0f);
				*/
				m_bodies[0]->SetAngularVelocity(-1.5f);
			}
			break;
		}
	}

	void Step(Settings* settings)
	{
		Body::Step(settings);

		PolyShapesCallback callback;
		callback.m_circle.m_radius = 2.0f;
		callback.m_circle.m_p.Set(0.0f, 2.1f);
		callback.m_transform.SetIdentity();
		callback.m_debugDraw = &m_debugDraw;

		b2AABB aabb;
		callback.m_circle.ComputeAABB(&aabb, callback.m_transform);

		m_world->QueryAABB(&callback, aabb);

		m_debugDraw.DrawString(5, m_textLine, "Press 1 to drop a ship");
		m_textLine += 15;

		if (m_bodies[0])
		{
			//const b2Fixture* fix = m_bodies[0]->GetFixtureList();
			//const b2AABB aabb = fix->GetAABB();
			//b2Vec2 v = m_bodies[0]->GetWorldCenter();
			b2Vec2 pos(m_bodies[0]->GetPosition());
			float32 angle = m_bodies[0]->GetAngle();
			if (pos.x > settings->viewport.x)
			{
				b2Vec2 newPos(-settings->viewport.x, pos.y);
				m_bodies[0]->SetTransform(newPos, angle);
			}
			else if (pos.x < -settings->viewport.x)
			{
				b2Vec2 newPos(settings->viewport.x, pos.y);
				m_bodies[0]->SetTransform(newPos, angle);
			}
			if (pos.y > settings->viewport.y)
			{
				b2Vec2 newPos(pos.x, -settings->viewport.y);
				m_bodies[0]->SetTransform(newPos, angle);
			}
			else if (pos.y < -settings->viewport.y)
			{
				b2Vec2 newPos(pos.x, settings->viewport.y);
				m_bodies[0]->SetTransform(newPos, angle);
			}
		}
		/*
		int32 lastDecelerate = glutGet(GLUT_ELAPSED_TIME) - m_lastDecelerate;
		int32 lastSpeedChange = glutGet(GLUT_ELAPSED_TIME) - m_lastSpeedChange;
		if (lastSpeedChange > 1000 && lastDecelerate > 250)
		{
			m_thrust -= 0.25f;
			m_thrust = b2Clamp(m_thrust, 0.0f, 100.0f);
			b2Vec2 vec(rad2vec(m_bodies[0]->GetAngle(), m_thrust));
			m_bodies[0]->SetLinearVelocity(vec);
			m_lastDecelerate = glutGet(GLUT_ELAPSED_TIME);
		}
		*/
	}

	static Body* Create()
	{
		return new Ship;
	}

	float m_thrust;
	//int32 m_lastSpeedChange;
	//int32 m_lastDecelerate;
	int32 m_bodyIndex;
	b2Body* m_bodies[k_maxBodies];
	b2PolygonShape m_polygons[4];
	b2CircleShape m_circle;
};

#endif
