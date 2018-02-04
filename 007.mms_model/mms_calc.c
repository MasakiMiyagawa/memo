#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

struct result {
	int32_t n_serv;	/* Num of service */
	double ta;	/* Average time of arrival */
	double ts;	/* Average time of service*/
	double lamda;	
	double myu;
	double rho;	
	double tw;	/* Average wait time (Tw) */
	double tf;	/* Time of invoke to finish (Ts + Tw) */
};

#define N_SERV_MAX 16
static struct result result[N_SERV_MAX];

static void dump_result(double ta, double ts, double lamda, double myu)
{
	int32_t i;
	printf("Average time of arrival (Ta) = %.4lf\n", ta);	
	printf("Average time of service (Ts) = %.4lf\n", ts);	
	printf("lamda (1/Ta) = %.4lf\n", lamda);
	printf("myu (1/Ts) = %.4lf\n", myu);
	printf("====================================================\n");
	printf("s\tmodel\trho\tTw\t\tTw+Ts\n");
	for (i = 0; i < N_SERV_MAX; i++) {
		char model[32];
		sprintf(model, "%s", 
			(result[i].n_serv == 1) ? "M/M/1" : "M/M/s");
		printf("%d\t%s\t%.5lf\t%.6lf\t%.6lf\n", 
			result[i].n_serv,
			model,
			result[i].rho,
			result[i].tw,
			result[i].tf);
	}
}

static void mm1(struct result *r)
{
	r->rho = r->lamda / r->myu;
	if (r->rho > 1.0f) {
		r->tw = -1.0f;
		r->tf = -1.0f;
		return;
	}

	r->tw = (r->rho / (1.0f - r->rho)) * r->ts;
	r->tf = r->tw + r->ts;
}

int main(int argc, char *argv[])
{
	double ave_ta = 0.0f;
	double ave_ts = 0.0f;
	double lamda = 0.0f;
	double myu = 0.0f;
	int32_t i;

	if (argc != 3) {
		fprintf(stderr, "USAGE: ./MMs-calc "
				" <Average time of arraival (min)>"
				" <Average time of service (min)>\n");
		exit(-1);
	}

	ave_ta = atof((const char *)argv[1]);
	ave_ts = atof((const char *)argv[2]);
	
	lamda = 1.0f / ave_ta;
	myu = 1.0f / ave_ts;

	memset((void *)result, 0, sizeof(result));
	for (i = 0; i <	N_SERV_MAX; i++) {
		struct result *r = &result[i];
		r->n_serv = (i + 1);
		r->lamda = lamda;
		r->myu = myu;
		r->ts = ave_ts;
		r->ta = ave_ta;
		if (r->n_serv == 1) {
			mm1(r);
		}
	}
	dump_result(ave_ta, ave_ts, lamda, myu);
	return 0;
}
