#include <stdio.h>

#include "hello_world.h"

void hello_world()
{
	printf("Hello, World!\n");
}

void hello_person(const char *name)
{
	printf("Hello, %s!\n", name);
}
