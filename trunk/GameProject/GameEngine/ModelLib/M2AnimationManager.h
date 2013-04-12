#pragma once 

struct ModelAnimation;
class M2Loader;

struct AnimInfo {
	short Loops;
	unsigned int AnimID;
};

class M2Animation {

public:
	M2Animation();
	~M2Animation();

	typedef std::vector<noMat4*> FrameList;
	typedef std::vector<mat4*>   FrameList2;


	void AddFrame(noMat4* frame) { frames.push_back(frame); }

	noMat4* GetFrameAt(float time);
	//noMat4* GetQuatFrameAt(float time);
	noMat4*	GetFrame(int index);
	int GetFrameIndexAt(float time);
	int GetNumFrames() { return (int)(frames.size()); }

	mat4*	GetFrame2(int index);
	mat4*	GetFrameAt2(float time);
	int GetFrameIndexAt2(float time);


	float duration;
	float timeStep;
		
	std::string name;

	int id;
	uint16 NextAnimation;
	bool bLooped;

	float moveSpeed;
	float playSpeed;

	FrameList frames;
	//FrameList quatFrames;

	FrameList2 frames2;
	//FrameList2 quatFrames2;

};

class M2AnimationManager {


protected:
	ModelAnimation *anims;

	bool Paused;
	bool AnimParticles;

	AnimInfo animList[4];

	size_t Frame;		// Frame number we're upto in the current animation
	size_t TotalFrames;

	ssize_t AnimIDSecondary;
	size_t FrameSecondary;
	size_t SecondaryCount;

	ssize_t AnimIDMouth;
	size_t FrameMouth;

	short Count;			// Total index of animations
	short PlayIndex;		// Current animation index we're upto
	short CurLoop;			// Current loop that we're upto.

	ssize_t TimeDiff;			// Difference in time between each frame

	float Speed;			// The speed of which to multiply the time given for Tick();
	float mouthSpeed;

	M2Animation* Animations;
		
	int numAnim_;

public:
	size_t currAnim;

public:
	M2AnimationManager(ModelAnimation *anim, int numAnims);
	~M2AnimationManager();

	void SetCount(int count);
	void AddAnim(unsigned int id, short loop); // Adds an animation to our array.
	virtual void SetAnim(unsigned int index, unsigned int id, short loop); // sets one of the 4 existing animations and changes it (not really used currently)

	void SetSecondary(int id) {
		AnimIDSecondary = id;
		FrameSecondary = anims[id].timeStart;
	}
	void ClearSecondary() { AnimIDSecondary = -1; }
	ssize_t GetSecondaryID() { return AnimIDSecondary; }
	size_t GetSecondaryFrame() { return FrameSecondary; }
	void SetSecondaryCount(int count) {	SecondaryCount = count; }
	size_t GetSecondaryCount() { return SecondaryCount; }

	// For independent mouth movement.
	void SetMouth(int id) {
		AnimIDMouth = id;
		FrameMouth = anims[id].timeStart;
	}
	void ClearMouth() { AnimIDMouth = -1; }
	ssize_t GetMouthID() { return AnimIDMouth; }
	size_t GetMouthFrame() { return FrameMouth; }
	void SetMouthSpeed(float speed) {
		mouthSpeed = speed;
	}

	virtual void Play(); // Players the animation, and reconfigures if nothing currently inputed
	virtual void Stop(); // Stops and resets the animation
	void Pause(bool force = false); // Toggles 'Pause' of the animation, use force to pause the animation no matter what.

	void Next(); // Plays the 'next' animation or loop
	void Prev(); // Plays the 'previous' animation or loop

	virtual int Tick(int time);

	size_t GetFrameCount();
	size_t GetFrame() {return Frame;}
	void SetFrame(size_t f);
	void SetSpeed(float speed) {Speed = speed;}
	float GetSpeed() {return Speed;}

	void SetLoop(short Loop) { CurLoop = Loop; }
	bool IsLoop() const { return CurLoop == 1;}


	void PrevFrame();
	void NextFrame();

	virtual void Clear();
	//void Reset() { Count = 0; }

	bool IsPaused() { return Paused; }
	bool IsParticlePaused() { return !AnimParticles; }
	void AnimateParticles() { AnimParticles = true; }

	size_t GetAnim() { return animList[PlayIndex].AnimID; }

	ssize_t GetTimeDiff();
	void SetTimeDiff(ssize_t i);
	void ProcessAnimations(M2Loader* pLoader);
	void extractFrames();

	M2Animation* GetAnim(unsigned int id) 
	{
		if (id == (unsigned int)-1)
			return NULL;
		return &Animations[id]; 
	}

	M2Animation* GetCurrAnim() 
	{
		if (currAnim ==  (unsigned int)-1)
			return NULL;
		return &Animations[currAnim]; 
	}
};
