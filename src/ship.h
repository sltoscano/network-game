
#ifndef SHIP_H
#define SHIP_H

#include "body.h"

#define  GLUT_KEY_LEFT                      0x0064
#define  GLUT_KEY_UP                        0x0065
#define  GLUT_KEY_RIGHT                     0x0066
#define  GLUT_KEY_DOWN                      0x0067


const int32 k_maxBodies = 256;

b2Color colors[] = {
	b2Color(1.00f, 1.00f, 0.60f),
	b2Color(1.00f, 0.80f, 0.60f),
	b2Color(0.80f, 1.00f, 0.60f),
	b2Color(0.80f, 0.40f, 0.60f),
	b2Color(0.60f, 1.00f, 0.60f),
	b2Color(0.60f, 0.20f, 0.60f),
	b2Color(0.40f, 1.00f, 0.60f),
	b2Color(0.40f, 0.60f, 0.60f),
	b2Color(0.20f, 0.00f, 0.60f),
	b2Color(0.00f, 0.40f, 0.20f)
};

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
		const b2Color& color = b2Color(0.5f, 0.5f, 0.5f);

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
	Ship(Settings* settings) : Body(settings)
	{
		// Shape of ship
		{
			b2Vec2 vertices[3];

			vertices[0].Set(-0.75f, -0.4f);
			vertices[1].Set(0.5f, 0.0f);
			vertices[2].Set(-0.75f, 0.4f);
			
			m_polygons[0].Set(vertices, 3);
		}

		m_nextID = 0;
		m_thrust = 0;
		m_bodyIndex = 0;
		memset(m_bodies, 0, sizeof(m_bodies));
	}

	~Ship()
	{
		DestroyBody();
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

		bd.position.Set(1.0f, 1.0f);
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

		int pos = m_nextID++ % (sizeof(colors)/sizeof(b2Color));
		for (b2Fixture* f = m_bodies[m_bodyIndex]->GetFixtureList(); f; f = f->GetNext())
		{
			f->SetUserData(new b2Color(colors[pos]));
		}

		m_bodyIndex = (m_bodyIndex + 1) % k_maxBodies;
	}

	void DestroyBody()
	{
		for (int32 i = 0; i < k_maxBodies; ++i)
		{
			if (m_bodies[i] != NULL)
			{
				for (b2Fixture* f = m_bodies[i]->GetFixtureList(); f; f = f->GetNext())
				{
					b2Color* pColor = (b2Color*) f->GetUserData();
					delete pColor;
				}
				m_world->DestroyBody(m_bodies[i]);
				m_bodies[i] = NULL;
			}
		}
	}

	b2Vec2 rad2vec(float r, float m = 1) {
		return b2Vec2(cosf(r)*m,sinf(r)*m);
	}

	void Keyboard(int key)
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
			}
			break;
		case GLUT_KEY_LEFT:
			if (m_bodies[0])
			{
				m_bodies[0]->SetAngularVelocity(1.5f);
			}
			break;
		case GLUT_KEY_RIGHT:
			if (m_bodies[0])
			{
				m_bodies[0]->SetAngularVelocity(-1.5f);
			}
			break;
		}
	}

	void Step()
	{
		Body::Step();

		PolyShapesCallback callback;
		callback.m_circle.m_radius = 2.0f;
		callback.m_circle.m_p.Set(m_settings->worldExtents.x / 2, m_settings->worldExtents.y / 2);
		callback.m_transform.SetIdentity();
		callback.m_debugDraw = &m_debugDraw;

		b2AABB aabb;
		callback.m_circle.ComputeAABB(&aabb, callback.m_transform);

		m_world->QueryAABB(&callback, aabb);

		b2Color color(0.4f, 0.7f, 0.8f);
		m_debugDraw.DrawCircle(callback.m_circle.m_p, callback.m_circle.m_radius, color);

		m_debugDraw.DrawString(5, m_textLine, "Press 1 to drop a ship");
		m_textLine += 15;
	}

	static Body* Create(Settings* settings)
	{
		return new Ship(settings);
	}

	float m_thrust;
	int32 m_nextID;
	int32 m_bodyIndex;
	b2Body* m_bodies[k_maxBodies];
	b2PolygonShape m_polygons[4];
	b2CircleShape m_circle;
};

#endif
