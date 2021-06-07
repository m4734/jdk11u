#ifndef SHARE_CGMIN_GLOBAL_DATA_HPP
#define SHARE_CGMIN_GLOBAL_DATA_HPP

#define HT_NUM 10000

class Method;

class GlobalData
{
	public:
	int test_m(Method* m,int bci);

	unsigned long now;
	unsigned long minor_GC_interval;

	// header-method + class

	// hash?

	int hash(int h,int c);


	unsigned long max_lt[HT_NUM];
	int max_sc[HT_NUM],current_sc[HT_NUM]; //sample count
	//pointer sample_object[HT_NUM];
};

#endif
