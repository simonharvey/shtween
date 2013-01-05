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
		.end()
	.get();
	
	while (!t2->complete()) {
		t2->update(.1f);
		cout << bar.foo << endl;
	}
	
    return 0;
}

