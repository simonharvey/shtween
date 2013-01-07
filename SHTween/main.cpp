//
//  main.cpp
//  SHTween
//
//  Created by Simon Harvey on 13-01-01.
//  Copyright (c) 2013 Simon Harvey. All rights reserved.
//

#include <iostream>
#include "shtween.h"

using namespace std;

struct Foo
{
	float x, y;
};

Foo operator-(const Foo& a, const Foo &b) {
	return (Foo){a.x-b.x, a.y-b.y};
}

Foo operator+(const Foo &a, const Foo &b) {
	return (Foo){a.x+b.x, a.y+b.y};
}

Foo operator*(const float &a, const Foo &b) {
	return (Foo){b.x*a, b.y*a};
}

Foo operator*(const Foo &b, const float &a) {
	return a*b;
}

Foo operator/(const Foo &a, const float &b) {
	return (Foo){a.x/b, a.y/b};
}

ostream &operator <<(ostream &o, const Foo &foo) {
	o << "Foo(" << foo.x << ", " << foo.y << ")";
	return o;
}

struct Bar
{
	Foo foo;
};

int main(int argc, const char * argv[])
{
	Bar bar = {100, 0};
	using namespace shtween;
	
	auto *t2 = TweenBuilder()
		.group<Sequence>()
			.to(bar.foo.x, 0.0f)
			.to(bar.foo.x, 100.0f)
			.group<Parallel>()
				.to(bar.foo.x, 0.0f)
				.to(bar.foo.y, 100.0f)
			.end()
			.to(bar.foo, (Foo){50.0f, 50.0f})
		.end()
	.get();
	
	while (!t2->complete()) {
		t2->update(.15f);
		cout << bar.foo << endl;
	}
	
    return 0;
}

