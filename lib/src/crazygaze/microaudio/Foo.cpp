#include "Foo.h"
#include <crazygaze/micromuc/StringUtils.h>

Foo::Foo()
{
}

Foo::~Foo()
{
}

static const char* gStr = "";
int Foo::add(int a, int b)
{
	gStr = cz::getFilename(__FILE__);
	return a+b;
}

