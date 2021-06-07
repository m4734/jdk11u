#include "cgmin/GlobalData.hpp"
#include "oops/method.hpp"

int GlobalData::test_m(Method *m,int bci)
{
	char s[100];
	m->name_and_sig_as_C_string(s,100);
	if (s[0] == 'j' && s[1] == 'a' && s[2] == 'v' && s[3] == 'a')
		return 0;
	return 1;
}

int GlobalData::hc_hash(int h,int c)
{
	return h + c;
}
