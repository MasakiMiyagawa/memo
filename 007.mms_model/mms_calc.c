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
	double wq;	/* Average wait time (Wq) */
	double tf;	/* Time of invoke to finish (Ts + Tw) */
};

#define N_SERV_MAX 64
static struct result result[N_SERV_MAX];

static void dump_result(double ta, double ts, double lamda, double myu)
{
	int32_t i;
	printf("Average time of arrival (Ta) = %.4lf\n", ta);	
	printf("Average time of service (Ts) = %.4lf\n", ts);	
	printf("lamda (1/Ta) = %.4lf\n", lamda);
	printf("myu (1/Ts) = %.4lf\n", myu);
	printf("====================================================\n");
	printf("s\tmodel\trho\t\tTw\t\tTw+Ts\n");
	for (i = 0; i < N_SERV_MAX; i++) {
		char model[32] = "M/M/s";
		if ((result[i].n_serv % 4) && (result[i].n_serv >= 4)) 
			continue;

		if (result[i].wq < 0.0f) {
			printf("%d\t%s\t%.6lf\t********\t*******\n", 
			result[i].n_serv,
			model,
			result[i].rho);
			continue;
		}

		printf("%d\t%s\t%.6lf\t%.6lf\t%.6lf\n", 
			result[i].n_serv,
			model,
			result[i].rho,
			result[i].wq,
			result[i].tf);
	}
}

static double my_kaijo(double n)
{
	if (n == 0)
		return 1;

	return n * my_kaijo(n - 1);
}

static double calc_B(struct result *r)
{
	int32_t i = 0;
	double ret = 0.0f;

	for (i = 0; i <= r->n_serv; i++) {
		double tmp = 0.0f;
		int32_t n = i;
		tmp = pow((r->n_serv * r->rho), n);
		tmp /= my_kaijo(n);
		ret += tmp;		
	}
	return ret;
}

static double calc_A(struct result *r)
{
	double deno = 0.0f;
	double mole = 0.0f;
	mole = pow((r->n_serv * r->rho), r->n_serv);
	deno = my_kaijo(r->n_serv);
	return (mole / deno);
}

static void mms(struct result *r)
{
	double A = 0.0f;
	double B = 0.0f;
	double P0 = 0.0f;
	double Pn = 0.0f;
	double uWq = 0.0f;

	r->rho = r->lamda / (r->myu * (double)r->n_serv);
	if (r->rho >= 1.0f) {
		r->wq = -1.0f;
		r->tf = -1.0f;
		return;
	}

	A = calc_A(r);
	B = calc_B(r);
	P0 = 1.0f / (B + A * (r->rho / (1.0f - r->rho)));
	Pn = (A / (1 - r->rho)) * P0;
	uWq = ((1.0f / (r->n_serv * (1.0f - r->rho))) * Pn);
	
	r->wq = uWq / r->myu;
	r->tf = r->wq + r->ts;
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
		mms(r);
	}
	dump_result(ave_ta, ave_ts, lamda, myu);
	return 0;
}
