//
//  shtween.h
//  SHTween
//
//  Created by Simon Harvey on 13-01-01.
//  Copyright (c) 2013 Simon Harvey. All rights reserved.
//

#ifndef SHTween_shtween_h
#define SHTween_shtween_h

#include <vector>
#include <limits>

using namespace std;

namespace shtween
{
	typedef void (*tween_callback_fun)();
	
	namespace easing
	{
		template <typename T>
		inline T linear(float t, const T &b, const T &c, float d) {
			return c*t/d + b;
		}
	}
	
	//
	
	struct Tween
	{
		virtual ~Tween() {}
		/**
		 Steps the tween by time dt, returns the amount of time left
		**/
		virtual float update(float dt) = 0;
		virtual bool complete() = 0;
	};
	
	struct TweenCallbacks
	{
		tween_callback_fun on_complete;
	};
	
	template <typename T>
	struct ValueTween : Tween
	{
		typedef T (*easing_fun)(float t, const T &b, const T &c, float d);
		
		easing_fun easing;
		float duration;
		float t;
		bool running;
		
		T &property;
		T start_value, end_value;
		
		ValueTween(T &property, T value, float duration = 1.0f) : property(property), running(false), end_value(value), duration(duration), t(0), easing(easing::linear<T>) {
			
		}
		
		virtual float update(float dt) {
			if (!running) {
				start_value = property;
				running = true;
			}
			t += dt;
			if (t <= duration) {
				property = easing(t, start_value, end_value-start_value, duration);
			} else {
				//t = duration;
				property = end_value;
			}
			return duration-t;
		}
		
		virtual bool complete() {
			return t >= duration;
		}
	};
	
	struct TweenGroup : Tween
	{
		vector<Tween *> children;
		
		virtual ~TweenGroup()
		{
			while (!children.empty()) {
				delete children.back();
				children.pop_back();
			}
		}
	};
	
	struct Sequence : TweenGroup
	{
		uint active_idx;
		
		Sequence() : active_idx(0) {}
		
		virtual float update(float dt)
		{
			if (complete()) return 0.0f;
			while ((dt = children[active_idx]->update(dt)) <= 0.0f) {
				++active_idx;
				if (complete()) return 0.0f;
			}
			return dt;
		}
	
		virtual bool complete() {
			return children.size() <= active_idx;
		}
	};
	
	struct Parallel : TweenGroup
	{
		bool m_complete;
		
		Parallel() : m_complete(false) {}
		
		virtual float update(float dt)
		{
			if (m_complete) return 0.0f;
			
			m_complete = true;
			float timeleft = std::numeric_limits<float>::min();
			
			for (auto *c : children) {
				if (!c->complete()) {
					timeleft = std::max(c->update(dt), timeleft);
					m_complete &= c->complete();
				}
			}
			
			return timeleft;
		}
		
		virtual bool complete()
		{
			return m_complete;
		}
	};
	
	//
		
	class TweenGroupBuilder
	{
	protected:
		TweenGroupBuilder *parent;
		TweenGroup *result;
		bool m_ownsResult;
		
	public:
		TweenGroupBuilder(TweenGroupBuilder *parent = NULL) : parent(parent), result(NULL) {
		
		}
		
		~TweenGroupBuilder() {
			if (m_ownsResult)
				delete result;
		}
		
		template <typename T>
		TweenGroupBuilder group()
		{
			TweenGroupBuilder b(this);
			b.result = new T();
			return b;
		}
		
		template <typename T>
		TweenGroupBuilder &to(T &property, T value, float duration = 1.0f)
		{
			result->children.push_back(new ValueTween<T>(property, value, duration));
			return *this;
		}
		
		TweenGroupBuilder &end()
		{
			if (parent && result)
				parent->end_child(*this);
			return *parent;
		}
		
		virtual Tween *get() {
			m_ownsResult = false;
			return result;
		}
		
		void end_child(TweenGroupBuilder &child) {
			result->children.push_back(child.get());
		}
	};
	
	
	class TweenBuilder : public TweenGroupBuilder
	{
	public:
		TweenBuilder() {
			result = new Sequence;
		}
	};
};

#endif
