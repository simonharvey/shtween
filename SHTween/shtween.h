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
	namespace easing
	{
		using std::pow;
		using std::cos;
		using std::sin;
	
#define PI		3.141592654
#define HALF_PI 1.570796327
		
		template <typename T>
		inline T linear(float t, const T &b, const T &c, float d) {
			return c*t/d + b;
		}
		
		template <typename T>
		inline T expo_in(float t, const T &b, const T &c, float d) {
			float p = pow(2, 10 * (t/d - 1));
			return (t==0) ? b : c * p + b;
		}

		template <typename T>
		inline T expo_out(float t, const T &b, const T &c, float d) {
			auto res = (t==d) ? b+c : c * (-(float)pow(2.0f, -10 * t/d) + 1) + b;
			return res;
		}
		
		template <typename T>
		inline T expo_in_out(float t, const T &b, const T &c, float d) {
			if (t == 0) return b;
			if (t == d) return b+c;
			if ((t/=d/2) < 1) return c/2.0f * (float)pow(2, 10*(t-1))+b;
			return c/2.0f * (-(float)pow(2, -10 * --t)+2) + b;
		}
		
		template <typename T>
		inline T sine_in(float t, const T &b, const T &c, float d) {
			return -c * cos(t/2.0f*HALF_PI) + c + b;
		}
		
		template <typename T>
		inline T sine_out(float t, const T &b, const T &c, float d) {
			return c * sin(t/d*HALF_PI) + b;
		}
		
		template <typename T>
		inline T sine_in_out(float t, const T &b, const T &c, float d) {
			return -c/2.0f * (float)(cos(PI * t/d) - 1) + b;
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
		
		ValueTween(T &property, T value, float duration = 1.0f, easing_fun easing = easing::linear<T>) : property(property), running(false), end_value(value), duration(duration), t(0), easing(easing) {
			
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
		TweenGroupBuilder &to(T &property, T value, float duration = 1.0f, typename ValueTween<T>::easing_fun easing = easing::linear)
		{
			result->children.push_back(new ValueTween<T>(property, value, duration, easing));
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
