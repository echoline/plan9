/* 
 * This file is part of the UCB release of Plan 9. It is subject to the license
 * terms in the LICENSE file found in the top-level directory of this
 * distribution and at http://akaros.cs.berkeley.edu/files/Plan9License. No
 * part of the UCB release of Plan 9, including this file, may be copied,
 * modified, propagated, or distributed except according to the terms contained
 * in the LICENSE file.
 */

#include "os.h"
#include <mp.h>
#include <libsec.h>

static void
genrand(mpint *p, int n)
{
	mpdigit x;

	// generate n random bits with high set
	mpbits(p, n);
	genrandom((uchar*)p->p, (n+7)/8);
	p->top = (n+Dbits-1)/Dbits;
	x = 1;
	x <<= ((n-1)%Dbits);
	p->p[p->top-1] &= (x-1);
	p->p[p->top-1] |= x;
}

RSApriv*
rsagen(int nlen, int elen, int rounds)
{
	mpint *p, *q, *e, *d, *phi, *n, *t1, *t2, *kp, *kq, *c2;
	RSApriv *rsa;

	p = mpnew(nlen/2);
	q = mpnew(nlen/2);
	n = mpnew(nlen);
	e = mpnew(elen);
	d = mpnew(0);
	phi = mpnew(nlen);

	// create the prime factors and euclid's function
	genstrongprime(p, nlen/2, rounds);
	genstrongprime(q, nlen - mpsignif(p) + 1, rounds);
	mpmul(p, q, n);
	mpsub(p, mpone, e);
	mpsub(q, mpone, d);
	mpmul(e, d, phi);

	// find an e relatively prime to phi
	t1 = mpnew(0);
	t2 = mpnew(0);
	genrand(e, elen);
	for(;;){
		mpextendedgcd(e, phi, d, t1, t2);
		if(mpcmp(d, mpone) == 0)
			break;
		mpadd(mpone, e, e);
	}
	mpfree(t1);
	mpfree(t2);

	// d = e**-1 mod phi
	mpinvert(e, phi, d);

	// compute chinese remainder coefficient
	c2 = mpnew(0);
	mpinvert(p, q, c2);

	// for crt a**k mod p == (a**(k mod p-1)) mod p
	kq = mpnew(0);
	kp = mpnew(0);
	mpsub(p, mpone, phi);
	mpmod(d, phi, kp);
	mpsub(q, mpone, phi);
	mpmod(d, phi, kq);

	rsa = rsaprivalloc();
	rsa->pub.ek = e;
	rsa->pub.n = n;
	rsa->dk = d;
	rsa->kp = kp;
	rsa->kq = kq;
	rsa->p = p;
	rsa->q = q;
	rsa->c2 = c2;

	mpfree(phi);

	return rsa;
}
