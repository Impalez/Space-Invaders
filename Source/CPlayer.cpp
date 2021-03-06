//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//       such as player movement, some minor physics as well as rendering.
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "CPlayer.h"
#include "CGameApp.h"


extern CGameApp g_App;
//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
CPlayer::CPlayer(const BackBuffer *pBackBuffer,const char* path) : rotateDirection(DIRECTION::DIR_FORWARD)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite(path, RGB(0xff, 0x00, 0xff));
	m_eSpeedState = SPEED_STOP;
	m_fTimer = 0;
	isDead = false;
	lives = 1;

	m_pSprite->setBackBuffer(pBackBuffer);

	// Animation frame crop rectangle
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 128;
	r.bottom = 128;

	m_pExplosionSprite	= new AnimatedSprite("data/explosion.bmp", "data/explosionmask.bmp", r, 16);
	m_bExplosion = false;
	m_iExplosionFrame = 0;

	m_pExplosionSprite->setBackBuffer( pBackBuffer );

	//Alien explosion
	RECT ra;
	ra.left = 0;
	ra.top = 0;
	ra.right = 128;
	ra.bottom = 128;

	m_eExplosionSprite = new AnimatedSprite("data/alienexplosion.bmp", "data/alienexplosionmask.bmp", ra, 16);
	m_beExplosion = false;
	m_ieExplosionFrame = 0;

	m_eExplosionSprite->setBackBuffer(pBackBuffer);
	
}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
CPlayer::~CPlayer()
{
	delete m_pSprite;
	delete m_pExplosionSprite;
	delete m_eExplosionSprite;
}

void CPlayer::Update(float dt)
{
	// Update sprite
	m_pSprite->update(dt);


	// Get velocity
	double v = m_pSprite->mVelocity.Magnitude();

	// NOTE: for each async sound played Windows creates a thread for you
	// but only one, so you cannot play multiple sounds at once.
	// This creation/destruction of threads also leads to bad performance
	// so this method is not recommanded to be used in complex projects.

	// update internal time counter used in sound handling (not to overlap sounds)
	m_fTimer += dt;

	// A FSM is used for sound manager 
	switch(m_eSpeedState)
	{
	case SPEED_STOP:
		if(v > 35.0f)
		{
			m_eSpeedState = SPEED_START;
			PlaySound("data/jet-start.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		break;
	case SPEED_START:
		if(v < 25.0f)
		{
			m_eSpeedState = SPEED_STOP;
			PlaySound("data/jet-stop.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		else
			if(m_fTimer > 1.f)
			{
				PlaySound("data/jet-cabin.wav", NULL, SND_FILENAME | SND_ASYNC);
				m_fTimer = 0;
			}
		break;
	}

	// NOTE: For sound you also can use MIDI but it's Win32 API it is a bit hard
	// see msdn reference: http://msdn.microsoft.com/en-us/library/ms711640.aspx
	// In this case you can use a C++ wrapper for it. See the following article:
	// http://www.codeproject.com/KB/audio-video/midiwrapper.aspx (with code also)
}

void CPlayer::Draw()
{
	if(!m_bExplosion && !m_beExplosion)
		m_pSprite->draw();
	else if(m_bExplosion)
	{
		m_pExplosionSprite->draw();
	}
	else if (m_beExplosion)
	{
		m_eExplosionSprite->draw();
	}
		
}

void CPlayer::Move(ULONG ulDirection)
{
	if (m_pSprite->mPosition.x - m_pSprite->width()/2 <= 0)
	{
		m_pSprite->mVelocity.x = 0;
		m_pSprite->mPosition.x = m_pSprite->mPosition.x + 1;
	}
	else if (ulDirection & CPlayer::DIR_LEFT)
	{
		m_pSprite->mVelocity.x -= 5;
	}

	if (m_pSprite->mPosition.x + m_pSprite->width()/2 >= GetSystemMetrics(SM_CXSCREEN))
	{
		m_pSprite->mVelocity.x = 0;
		m_pSprite->mPosition.x = m_pSprite->mPosition.x - 1;
	}
	else if (ulDirection & CPlayer::DIR_RIGHT)
	{
		m_pSprite->mVelocity.x += 5;
	}

	if (m_pSprite->mPosition.y - m_pSprite->height()/2 <= 0)
	{
		m_pSprite->mVelocity.y = 0;
		m_pSprite->mPosition.y = m_pSprite->mPosition.y + 1;
	}
	else if (ulDirection & CPlayer::DIR_FORWARD)
	{
		m_pSprite->mVelocity.y -= 5;
	}

	if (m_pSprite->mPosition.y + m_pSprite->height() >= GetSystemMetrics(SM_CYSCREEN) - m_pSprite->height()/3 - 40)
	{
		m_pSprite->mVelocity.y = 0;
		m_pSprite->mPosition.y = m_pSprite->mPosition.y - 1;
	}
	else if (ulDirection & CPlayer::DIR_BACKWARD)
	{
		m_pSprite->mVelocity.y += 5;
	}
		
}

Vec2& CPlayer::Position()
{
	return m_pSprite->mPosition;
}

Vec2& CPlayer::Velocity()
{
	return m_pSprite->mVelocity;
}

void CPlayer::Explode()
{
	m_pExplosionSprite->SetFrame(0);
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pSprite->mVelocity.x = 0;
	m_pSprite->mVelocity.y = 0;
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}

bool CPlayer::AdvanceExplosion()
{
	if(m_bExplosion)
	{
		m_pExplosionSprite->SetFrame(m_iExplosionFrame++);
		
		if(m_iExplosionFrame==m_pExplosionSprite->GetFrameCount())
		{
			isDead = true;
			m_bExplosion = false;
			m_iExplosionFrame = 0;
			m_eSpeedState = SPEED_STOP;
			return false;
		}
	}

	return true;
}

int& CPlayer::frameCounter() {
	return m_pSprite->frameCounter;
}

Vec2 CPlayer::getSize()
{
	return Vec2(m_pSprite->width(), m_pSprite->height());
}


void CPlayer::EnemyExplode()
{
	m_eExplosionSprite->SetFrameEnemy(0);
	m_eExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pSprite->mVelocity.x = 0;
	m_pSprite->mVelocity.y = 0;
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_beExplosion = true;
}

bool CPlayer::EnemyAdvanceExplosion()
{
	if (m_beExplosion)
	{
		m_eExplosionSprite->SetFrameEnemy(m_ieExplosionFrame++);
		if (m_ieExplosionFrame == m_eExplosionSprite->GetFrameCount())
		{
			isDead = true;
			m_beExplosion = false;
			m_ieExplosionFrame = 0;
			m_eSpeedState = SPEED_STOP;
			return false;
		}
	}

	return true;
}

void CPlayer::takeDamage()
{
	lives--;
}

int CPlayer::getLives()
{
	return lives;
}

void CPlayer::setLives(int noLives)
{
	lives = noLives;
}

bool CPlayer::hasExploded()
{
	return m_bExplosion;
}

bool CPlayer::enemyHasExploded()
{
	return m_beExplosion;
}

void CPlayer::Rotate(int x)
{
	auto position = m_pSprite->mPosition;
	auto velocity = m_pSprite->mVelocity;

	switch (rotateDirection)
	{
	case CPlayer::DIR_FORWARD:
		if (x == 1)
		{
			m_pSprite = new Sprite("data/ship1r.bmp", RGB(0xff, 0x00, 0xff));
			rotateDirection = CPlayer::DIR_LEFT;
		}
		else
		{
			m_pSprite = new Sprite("data/ship2r.bmp", RGB(0xff, 0x00, 0xff));
			rotateDirection = CPlayer::DIR_LEFT;
		}
		break;
	case CPlayer::DIR_BACKWARD:
		if (x == 1)
		{
			m_pSprite = new Sprite("data/ship1rrr.bmp", RGB(0xff, 0x00, 0xff));
			rotateDirection = CPlayer::DIR_RIGHT;
		}
		else
		{
			m_pSprite = new Sprite("data/ship2rrr.bmp", RGB(0xff, 0x00, 0xff));
			rotateDirection = CPlayer::DIR_RIGHT;
		}
		break;
	case CPlayer::DIR_LEFT:
		if (x == 1)
		{
			rotateDirection = CPlayer::DIR_BACKWARD;
			m_pSprite = new Sprite("data/ship1rr.bmp", RGB(0xff, 0x00, 0xff));
		}
		else
		{
			rotateDirection = CPlayer::DIR_BACKWARD;
			m_pSprite = new Sprite("data/ship2rr.bmp", RGB(0xff, 0x00, 0xff));
		}
		break;
	case CPlayer::DIR_RIGHT:
		if (x == 1)
		{
			rotateDirection = CPlayer::DIR_FORWARD;
			m_pSprite = new Sprite("data/ship1.bmp", RGB(0xff, 0x00, 0xff));
		}
		else
		{
			rotateDirection = CPlayer::DIR_FORWARD;
			m_pSprite = new Sprite("data/ship2.bmp", RGB(0xff, 0x00, 0xff));
		}
		break;
	}

	m_pSprite->mPosition = position;
	m_pSprite->mVelocity = velocity;
	m_pSprite->setBackBuffer(g_App.m_pBBuffer);
}

void CPlayer::SetLives(int currentLive) {
	lives = currentLive;
}

void CPlayer::SetPosition(Vec2 currentPosition) {
	m_pSprite->mPosition = currentPosition;
}