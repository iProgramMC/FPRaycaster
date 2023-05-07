#include "Raycaster.hpp"
int main()
{
	Raycaster r;
	if (r.Construct(1280, 720, 1, 1))
		r.Start();
	return 0;
}