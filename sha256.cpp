#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <array>
#include <algorithm>

using namespace std;
#define INT_BITS 32

template <std::size_t N>
bitset<N> rightRotate(bitset<N> n, int d);
template <std::size_t N>
bitset<N> leftRotate(bitset<N> n, int d);
template <size_t N>
bitset<N > bitsetAdd(bitset<N>& x, bitset<N>& y);

bitset<32>* generate_message_schedule(vector<bool> chunk_vector);
vector<bool> generate_input_vector(string input);

bitset<256> encode(string input);

int intleftRotate(int n, int d);
int intrightRotate(int n, int d);

template <size_t N>
bitset<N> bitsetReverse(bitset<N>& x);

int main(void) {
	//Get input message string
	string input;
	cout << "Enter message: ";
	cin >> input;

	//Encode
	bitset<256> result = encode(input);

	//Print result as binary
	cout << endl << "Result: " << endl;
	for (int i = 0; i < result.size(); i++) {
		cout << result[i];
	}

	return 0;
}

bitset<256> encode(string input) {
	//First 32 bits of fractional parts of square roots of first 8 primes
	unsigned int h0 = 0x6a09e667;
	unsigned int h1 = 0xbb67ae85;
	unsigned int h2 = 0x3c6ef372;
	unsigned int h3 = 0xa54ff53a;
	unsigned int h4 = 0x510e527f;
	unsigned int h5 = 0x9b05688c;
	unsigned int h6 = 0x1f83d9ab;
	unsigned int h7 = 0x5be0cd19;

	//First 32 bits of fractional parts of cube roots of first 64 primes
	unsigned int k[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	//Generate message schedule
	vector<bool> input_vector = generate_input_vector(input);
	bitset<32>* w = generate_message_schedule(input_vector);


	//Working variables
	unsigned int a = h0;
	unsigned int b = h1;
	unsigned int c = h2;
	unsigned int d = h3;
	unsigned int e = h4;
	unsigned int f = h5;
	unsigned int g = h6;
	unsigned int h = h7;


	//Compression Function
	for (int i = 0; i < 64; i++) {
		int S1 = intrightRotate(e, 6) ^ intrightRotate(e, 11) ^ intrightRotate(e, 25);
		int ch = (e & f) ^ (~e & g);
		int temp1 = h + S1 + ch + k[i] + static_cast<int>(w[i].to_ulong());
		int S0 = intrightRotate(a, 2) ^ intrightRotate(a, 13) ^ intrightRotate(a, 22);
		int maj = (a & b) ^ (a & c) ^ (b & c);
		int temp2 = S0 + maj;

		h = g;
		g = f;;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}

	h0 = h0 + a;
	h1 = h1 + b;
	h2 = h2 + c;
	h3 = h3 + d;
	h4 = h4 + e;
	h5 - h5 + f;
	h6 = h6 + g;
	h7 = h7 + h;

	//Append all the bits into a hash
	array<uint32_t, 8> hs = { h0, h1, h2, h3, h4, h5, h6, h7 };
	bitset<256> result;

	for (size_t i = 0; i < hs.size(); ++i) {
		std::bitset<32> bits(hs[i]);
		for (size_t j = 0; j < 32; ++j) {
			result[(7 - i) * 32 + j] = bits[j];
		}
	}

	return result;

}

vector<bool> generate_input_vector(string input) {
	vector<bool> input_vector;
	for (char c : input) {
		bitset<8> bits(c);

		//Bitsets have index backwards - MSB is at index 7
		for (int i = 7; i >= 0; i--) {
			input_vector.push_back(bits[i]);
		}
	}

	input_vector.push_back(1);

	//Add bits to make input multiple of 512
	int extra_bits = 0;
	while ((input.length() * 8 + 1 + extra_bits + 64) % 512 != 0) {
		input_vector.push_back(0);
		extra_bits++;
	}

	//Append message length as 64 bit number
	bitset<64> size_number(input.length() * 8);
	for (int i = 63; i >= 0; i--) {
		input_vector.push_back(size_number[i]);
	}

	return input_vector;
}

bitset<32>* generate_message_schedule(vector<bool> chunk_vector) {
	size_t s = 64 * sizeof(bitset<32>);
	bitset<32>* w = (bitset<32>*)malloc(s);

	//Set first 16 rows to be identical to chunk
	int count = 0;
	int word = 0;
	for (bool bit : chunk_vector) {
		w[word].set(count, bit);
		count++;

		if (!(count % 32)) {
			word++;
			count = 0;
		}
	}

	cout << endl;

	cout << "Message schedule 1: " << endl;
	for (int j = 0; j < 64; j++) {
		bitset<32> p_vector = w[j];
		for (int i = 0; i < 32; i++) {
			cout << p_vector[i];
		}
		cout << endl;
	}


	//Set rest rows according to algorithm
	for (int i = 16; i < 64; i++) {
		//Calculate sig0
		bitset<32> word = i - 15 > 15 ? w[i - 15] : bitsetReverse(w[i - 15]);

		bitset<32> rrotate_7 = rightRotate(word, 7);
		bitset<32> rrotate_18 = rightRotate(word, 18);
		bitset<32> rshift_3 = word >> 3;

		//cout << "word: " << word << endl;
		//cout << "rrotate_7: " << rrotate_7 << endl;
		//cout << "rrotate_18: " << rrotate_18 << endl;
		//cout << "rshift_3: " << rshift_3 << endl;


		bitset<32> sig_0 = rrotate_7 ^ rrotate_18 ^ rshift_3;
		//cout << "sig_0: " << sig_0 << endl;

		//Calculate sig1
		word = i - 2 > 15 ? w[i - 2] : bitsetReverse(w[i - 2]);

		bitset<32> rrotate_17 = rightRotate(word, 17);

		bitset<32> rrotate_19 = rightRotate(word, 19);
		bitset<32> rshift_10 = word >> 10;

		/*cout << "word: " << word << endl;
		cout << "rrotate_17: " << rrotate_17 << endl;
		cout << "rrotate_19: " << rrotate_19 << endl;
		cout << "rshift_10: " << rshift_10 << endl;*/

		bitset<32> sig_1 = rrotate_17 ^ rrotate_19 ^ rshift_10;





		//Set value of word
		bitset<32> b1 = i - 16 > 15 ? w[i - 16] : bitsetReverse(w[i - 16]);
		bitset<32> b2 = i - 7 > 15 ? w[i - 7] : bitsetReverse(w[i - 7]);

		/*bitset<32> b1 = bitsetReverse(w[i - 16]);
		bitset<32> b2 = bitsetReverse(w[i - 7]);*/

		cout << "i: " << i << endl;
		cout << "b1: " << b1 << endl;
		cout << "sig_0: " << sig_0 << endl;
		cout << "b2: " << b2 << endl;
		cout << "sig_1: " << sig_1 << endl;

		bitset<32> sum1 = bitsetAdd(b1, sig_0);
		bitset<32> sum2 = bitsetAdd(b2, sig_1);
		w[i] = bitsetAdd(sum1, sum2);

		cout << "w[i] " << w[i] << endl;

		if (i > 15) {
			w[i] = bitsetReverse(w[i]);
		}

		cout << "w[i] " << w[i] << endl;

		cout << endl;


	}

	cout << "Message schedule 2: " << endl;
	for (int j = 0; j < 64; j++) {
		bitset<32> p_vector = w[j];
		cout << j << ": ";
		for (int i = 0; i < 32; i++) {
			cout << p_vector[i];
		}
		cout << endl;
	}

	cout << endl;

	return w;
}

template <size_t N>
bitset<N> rightRotate(bitset<N> n, int d) {
	return (n >> d) | (n << (N - d));
}

template <size_t N>
bitset<N> leftRotate(bitset<N> n, int d) {
	return (n << d) | (n >> (N - d));
}

int intrightRotate(int n, int d) {
	return (n >> d) | (n << (INT_BITS - d));
}

int intleftRotate(int n, int d) {
	return (n << d) | (n >> (INT_BITS - d));
}

template <size_t N>
bitset<N> bitsetAdd(bitset<N>& x, bitset<N>& y)
{
	unsigned int num1 = static_cast<unsigned int>(x.to_ulong());
	unsigned int num2 = static_cast<unsigned int>(y.to_ulong());

	int ans = num1 + num2;

	bitset<N> answer(ans);

	return answer;
}

template <size_t N>
bitset<N> bitsetReverse(bitset<N>& x) {
	bitset<N> result = x;
	for (int i = 0; i < N; i++) {
		result[N - i - 1] = x[i];
	}

	return result;
}