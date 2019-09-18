#include <cmath>
  int main()
  {
    const int size = 256;
    double sinTable[size];
	for (int i =0; i <999999999;i++){    
    #pragma omp parallel for
    for(int n=0; n<size; ++n)
      sinTable[n] = std::sin(2 * M_PI * n / size);
  }
    // the table is now initialized
  }

