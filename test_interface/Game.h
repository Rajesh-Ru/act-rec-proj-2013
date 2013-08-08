#pragma once
#include <vector>
#include <unordered_set>
#include "Broadcast.h"
class Random{
public:
	Random();
	int operator()()const;
};
class Game{
public:
	friend struct GameInternal;
	struct TPlayer{
		enum PlayerState{
			waitting = 0,
			running,
			jumping,
			slipping,
			hitting,
			hitwall,
		} state;
		float z;
		enum RunState{
			go,
			turnleft,
			turnright,
			gostraight,
			left,
			right,
			straight,
		} runstate;
		// position ¡Ê [0, 1) 
		float position;
		// offset ¡Ê [-0.5, 0.5]
		float offset;
		enum NextDirection{
			d_straight,
			d_left,
			d_right,
		} nextdir;
	};
	enum RoadType{
		none = 0,
		straight=1,
		left=2,
		right=4,
		straightleft = straight | left,
		straightright = straight | right,
		leftright = left | right,
		all = straight | left | right,
	};
	enum BarrierType{
		fire,
		hole,
		tree,
		gold,
	};
	struct Barrier{
		BarrierType Type;
		// x, y ¡Ê [0, 3] 
		float x, y;
	};
	enum GameState{
		initializing,
		ready,
		running,
		end,
	};
protected:
	struct Road{
		static int const Deep = 3;
		Road *straight, *left, *right;
		int roadid;
		std::vector<Barrier> barriers;
		inline RoadType Type()const{
			return RoadType(
				(straight ? RoadType::straight : RoadType::none) |
				(left ? RoadType::left : RoadType::none) |
				(right ? RoadType::right : RoadType::none)
			);
		}
	} *curroad, *pre;
	RoadType pretype;
	TPlayer player;
	GameState state;
	Random random;
	std::unordered_set<Road*> border;
	bool isend;
public:
	inline bool IsEnd()const{
		return isend;
	}
	inline void End(){
		isend = true;
		state = GameState::end;
		OnFinishGame(*this);
	}
	float RoadSize;
	Game(std::function<void(Game&)> initializer = [](Game&){});
	~Game();
	enum ResultCode{
		Succeed = 0,
	};
	ResultCode update();
	TPlayer const& Player()const;
	GameState State()const;
	Broadcast<void(Game const&)> OnInitialized;
	Broadcast<void(Game&)> OnFinishGame;
	Broadcast<void(Game const&)> BeforeDrawRoad;
	Broadcast<void(Game const&, RoadType, int RoadID, std::vector<Barrier> const& Barriers)> OnDrawRoad;
	Broadcast<void(Game const&)> AfterDrawRoad;
	Broadcast<void(Game const&, TPlayer const&)> OnDrawPlayer;
	std::function<RoadType(Game const&, int RoadID, std::vector<Barrier>& barriers, RoadType mask)> OnCreateRoad;
	std::function<void(Game const&, TPlayer&, float x, float y, std::vector<Barrier>&)> OnPlayerUpdate;
	Broadcast<void(Game const&, TPlayer&, float x, float y)> OnHitWall;
	bool TryTurnLeft();
	bool TryTurnRight();
	void Move(float delta);
	void GoStraight();
};
