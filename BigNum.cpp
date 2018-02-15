
/*
Made by Lars Thomas Bremnes

/unlicense
*/


#include <string>


#ifndef BigNumElementSize
	typedef	  unsigned __int64 BigNumElement;
	constexpr unsigned __int64 MaxVal = (0xFFFFFFFFFFFFFFFF);
#else
	#if BigNumElementSize == 1
		typedef   unsigned char BigNumElement;
		constexpr unsigned char MaxVal = 0xFF;
	#elif BigNumElementSize == 2
		typedef   unsigned short BigNumElement;
		constexpr unsigned short MaxVal = (0xFFFF);
	#elif BigNumElementSize == 4
		typedef   unsigned int BigNumElement;
		constexpr unsigned int MaxVal = (0xFFFFFFFF);
	#else
		void YaDidItWrong(WhyWouldYouDoThat You) {
			return Fool;
		}
	#endif
#endif

struct BigNum {
	private:
	unsigned __int64	LogicalSize;
	unsigned __int64	MemoryCapacity;
	BigNumElement* 		n;



	enum bignum_limits: unsigned __int64 {
		U64Lim = 0xFFFFFFFFFFFFFFFF
	};

	enum bignum_mem_expansion_slack: unsigned __int64 {
		Slack = 5
	};

	void ExpandTo(unsigned __int64 In) {
		BigNumElement* New = (BigNumElement*) calloc(In, sizeof(BigNumElement));
		memcpy(New, n, LogicalSize*sizeof(BigNumElement));
		free(n);
		n = New;
		MemoryCapacity = In;
	}

	struct OverflowDiagnosis {
		unsigned long		Bit1;
		unsigned long		Bit2;
		unsigned __int64	a;
		BigNumElement		i;
		BigNumElement		j;
		BigNumElement		OverflownValue;

		void FindIndexesOfSetBits(
			BigNumElement Indexes[8*sizeof(BigNumElement)],
			BigNumElement Val
		) {
			BigNumElement Mask = 1;
			unsigned __int64 i = 0;
			unsigned __int64 Index = 1;
			while (Mask) {
				if (Val & Mask) {
					Indexes[i++] = Index;
				}
				Mask <<= 1;
				Index++;
			}
		}

		void CalculateOverflow() {
			this->a = this->i*this->j;
			BigNumElement i = this->i, j = this->j;
			BigNumElement Bits[(sizeof(BigNumElement))*16] = {};

			BitScanReverse64(&this->Bit1, i);
			BitScanReverse64(&this->Bit2, j);
			BigNumElement Overflow = this->Bit1 + this->Bit2;

			if (Overflow < (sizeof(BigNumElement)*8 - 1)) {
				this->OverflownValue = 0;
				return;
			}

			BigNumElement Indexes[(sizeof(BigNumElement))*8 + 1] = {};
			FindIndexesOfSetBits(Indexes, this->j);
			while (i) {
				unsigned __int64 k = 0;
				while (Indexes[k]) {
					unsigned __int64 BitIndex = (Indexes[k] - 1) + this->Bit1;
					Bits[BitIndex] += 1;
					unsigned __int64 Onwards = BitIndex;
					while (Bits[Onwards] > 1) {
						Bits[Onwards + 1]++;
						Bits[Onwards] -= 2;
						Onwards++;
					}
					k++;
				}
				i -= (1ULL << this->Bit1);
				BitScanReverse64(&this->Bit1, i);
			}

			for (int index = 8*sizeof(BigNumElement); index < (sizeof(BigNumElement)*16); index++) {
				this->OverflownValue += Bits[index] << (index - 8*sizeof(BigNumElement));
			}
		}
	};

	std::string AddStrings(std::string a, std::string b) {
		std::string	S, LongStr, ShortStr;
		unsigned __int64 ALen = a.length(), BLen = b.length();
		unsigned __int64 LongIndex, ShortIndex, EqualLength = 0;
		if (ALen > BLen) {
			LongStr = a;
			ShortStr = b;
			LongIndex = ALen - 1;
			ShortIndex = BLen - 1;
		} else {
			LongStr = b;
			ShortStr = a;
			LongIndex = BLen - 1;
			ShortIndex = ALen - 1;
			if (ALen == BLen) {
				EqualLength = 1;
			}
		}
		unsigned char Carry = 0;
		S.reserve(LongIndex);
		for (; ShortIndex != U64Lim; ShortIndex--, LongIndex--) {
			char c = (LongStr[LongIndex] - '0') + (ShortStr[ShortIndex] - '0') + Carry;
			if (c > 9) {
				c -= 10;
				Carry = 1;
			} else {
				Carry = 0;
			}
			S.insert(0, std::to_string(c));
		}
		if (EqualLength == 0) {
			if (Carry) {
				S.insert(0, LongStr, 0, LongIndex + 1);
				S[LongIndex]++;
				while (S[LongIndex] > '9') {
					S[LongIndex] = '0';
					if (LongIndex == 0) {
						char Buffer[2] = "1";
						S.insert(0, Buffer);
					} else {
						S[LongIndex - 1]++;
					}
					LongIndex--;
				}
			} else {
				S.insert(0, LongStr, 0, LongIndex + 1);
			}
		} else {
			if (Carry) {
				S.insert(0, "1");
			}
		}
		return S;
	}

	std::string MultiplyStrings(std::string a, std::string b) {
		__int64 Carry = 0;
		std::string  S;
		if (!(a.compare("0") && b.compare("0"))) {
			S.insert(0, std::to_string(0));
			return S;
		}
		__int64 CurLength = 0;
		for (__int64 i = a.length() - 1, I1 = 0; i >= 0; i--, I1++) {
			for (__int64 j = b.length() - 1, I2 = 0; j >= 0; j--, I2++) {
				__int64 Val = (a[i] - '0')*(b[j] - '0');
				Carry = Val / 10;
				if (Carry) {
					Val -= 10*Carry;
				}
				if ((I1 + I2) < CurLength) {
					S[CurLength - 1 - (I1 + I2)] += Val;
					if (S[CurLength - 1 - (I1 + I2)] > '9') {
						S[CurLength - 1 - (I1 + I2)] -= 10;
						Carry += 1;
					}
					__int64 CI = 1;
					while (Carry) {
						if ((I1 + I2 + CI) < CurLength) {
							S[CurLength - 1 - (I1 + I2 + CI)] += Carry;
							Carry = 0;
							if (S[CurLength - 1 - (I1 + I2 + CI)] > '9') {
								S[CurLength - 1 - (I1 + I2 + CI)] -= 10;
								Carry = 1;
							}
							CI++;
						} else {
							S.insert(0, std::to_string(Carry));
							CurLength++;
							Carry = 0;
						}
					}
				} else {
					S.insert(0, std::to_string(Val));
					CurLength++;
					if (Carry) {
						S.insert(0, std::to_string(Carry));
						CurLength++;
						Carry = 0;
					}
				}
			}
		}
		if (Carry) {
			S.insert(0, std::to_string(Carry));
		}
		return S;
	}


	// Optimizes ToString() by creating a lookup-table so that the factors are only
	// calculated once and then stored in Factors up until max capacity (which you
	// can modify yourself), allowing a single read to reach the next one. Choosing
	// this optimization increases the size of a BigNum by something like 32 bytes
	// (it varies) multiplied by FactorStorageLimit, (so for 10, the default value,
	// the final size will be ~350 bytes) providing up to a 53 % speedup if
	// FactorStorageLimit is large enough (10 is usually plenty, maybe even overkill).

	#ifdef OptimizeToString
	#define FactorStorageLimit 10
	std::string  Factors[FactorStorageLimit];
	unsigned __int64 FactorsIndex = 0;
	#endif

	public:

	static constexpr const size_t GetElementSize() {
		return sizeof(BigNumElement);
	}

	BigNum() {
		n = (BigNumElement*) calloc(10, sizeof(BigNumElement));
		MemoryCapacity = 10;
		LogicalSize = 0;
	}

	BigNum(unsigned __int64 InitialMemorySize) {
		n = (BigNumElement*) calloc(InitialMemorySize, sizeof(BigNumElement));
		MemoryCapacity = InitialMemorySize;
		LogicalSize = 0;
	}

	void Set(BigNumElement i) {
		if (MemoryCapacity == 0) {
			n = (BigNumElement*) calloc(10, sizeof(BigNumElement));
			MemoryCapacity = 10;
		} else {
			memset(n, 0, MemoryCapacity*sizeof(BigNumElement));
		} n[0] = i;
		LogicalSize = 1;
	}

	void Set(BigNum B) {
		if (LogicalSize < B.LogicalSize) {
			if (MemoryCapacity < B.MemoryCapacity) {
				ExpandTo(B.MemoryCapacity);
			} memcpy(n, B.n, B.LogicalSize*GetElementSize());
		} else {
			memcpy(n, B.n, B.LogicalSize*GetElementSize());
			memset(n + B.LogicalSize, 0, (LogicalSize - B.LogicalSize)*GetElementSize());
		} LogicalSize = B.LogicalSize;
	}


	//#if BigNumElementSize == 8
	void Set(std::string S) {
		unsigned char	 FullHam = 0;
		unsigned __int64 Length  = S.length();
		if (Length < 20) {
			ReEntry:
			unsigned __int64 Val = 0;
			unsigned __int64 Ten = 1;
			for (unsigned __int64 i = (Length - 1); i != U64Lim; i--) {
				Val += (S[i] - '0')*Ten;
				Ten *= 10;
			}
			Set(Val);
			return;
		} else if (Length == 20 && S[0] == '1') {
			std::string Compare = "18446744073709551615";
			for (unsigned __int64 i = 1; i < 20; i++) {
				__int64 Val = S[i] - Compare[i];
				if (Val > 0) {
					FullHam = 1;
					break;
				}
			}
		} else {
			FullHam = 1;
		}

		if (FullHam) {
			BigNum Val; Val.Set(0);
			BigNum Ten; Ten.Set(1);
			BigNum Temp;
			for (unsigned __int64 i = (Length - 1); i != U64Lim; i--) {
				Temp.Set(S[i] - '0');
				Temp.MultiplySelf(Ten);
				Val.Add(Temp);
				Ten.MultiplySelf(10);
			}
			Set(Val);
			Val.Clear();
			Ten.Clear();
			Temp.Clear();
		} else {
			goto ReEntry;
		}
	}
	//#endif


	void Clear() {
		if (n) {
			free(n);
		}

		#ifdef OptimizeToString
		for (std::string s : Factors) {
			s.clear();
		}
		#endif

		memset(this, 0, sizeof(BigNum));
	}

	// Very slow. Use as rarely as possible.
	std::string ToString() {
		if (LogicalSize == 0) return 0;
		unsigned __int64 Index = 0;
		std::string  Factor = "1";
		std::string  LeftShiftBy =


			#ifndef BigNumElementSize
				"18446744073709551616"
			#else
				#if BigNumElementSize == 1
					"256"
				#elif BigNumElementSize == 2
					"65536"
				#elif BigNumElementSize == 4
					"4294967296"
				#endif
			#endif
		;
		std::string S = "0";
		while (Index <= (LogicalSize - 1)) {
			unsigned __int64 Val = n[Index++];
			std::string Cur = std::to_string(Val);
			Cur = MultiplyStrings(Factor, std::to_string(Val));
			S = AddStrings(Cur, S);
			#ifdef OptimizeToString
			if (Index - 1 < FactorStorageLimit) {
				if (Index == (FactorsIndex + 1)) {
					Factors[FactorsIndex++] = MultiplyStrings(Factor, LeftShiftBy);
				}
				Factor = Factors[Index - 1];
			} else
				#endif
				Factor = MultiplyStrings(Factor, LeftShiftBy);
	}
		return S;
		}

	// From (and including) Start, to (but not including) End. Treated as indices.
	// Expands as necessary.
	void SetValOverRange(BigNumElement Val, unsigned __int64 Start, unsigned __int64 End) {
		if (MemoryCapacity < (End)) {
			ExpandTo(End + bignum_mem_expansion_slack::Slack);
		} if (LogicalSize < (End + 1)) {
			LogicalSize = End;
		}

		for (unsigned __int64 i = Start; i < End; i++) {
			n[i] = Val;
		}
	}

	// Expands as necessary.
	void SetValAt(BigNumElement Val, unsigned __int64 Index) {
		if (MemoryCapacity < Index) {
			ExpandTo(Index + bignum_mem_expansion_slack::Slack);
		} if (LogicalSize < (Index + 1)) {
			LogicalSize = Index + 1;
		}

		n[Index] = Val;
	}


	// a += b
	void Add(BigNumElement In) {
		if (MemoryCapacity == 0) {
			DebugBreak();
		} if (this->n[0] > (MaxVal - In)) {
			this->n[0] += In;
			BigNumElement Overflow = 1;
			unsigned __int64 i = 1;
			while (Overflow) {
				if (i == MemoryCapacity) {
					ExpandTo(MemoryCapacity + 10);
				} if (i == LogicalSize) {
					LogicalSize++;
				}
				if (this->n[i] > (MaxVal - Overflow)) {
				} else {
					Overflow = 0;
				}
				this->n[i++]++;
			}
		} else {
			this->n[0] += In;
			if (LogicalSize == 0) LogicalSize++;
		}
	}


	// BigNum a += b;
	void Add(BigNum B) {
		if (!B.LogicalSize) DebugBreak();
		if (B.LogicalSize > LogicalSize) {
			ExpandTo(B.LogicalSize);
		}
		this->Add(B.n[0]);
		for (unsigned __int64 i = 1; i < B.LogicalSize; i++) {
			if (this->n[i] >(MaxVal - B.n[i])) {
				this->n[i] += B.n[i];
				BigNumElement Overflow = 1;
				unsigned __int64	   i = 1;
				while (Overflow) {
					LogicalSize++;
					if (this->n[i] > (MaxVal - Overflow)) {} else {
						Overflow = 0;
					}
					this->n[i++]++;
				}
			} else {
				this->n[i] += B.n[i];
			} if ((i + 1) > LogicalSize) {
				LogicalSize++;
			}
		}
	}

	//BigNum a = b + c
	static BigNum Add(BigNum A, BigNumElement In) {
		BigNum R; R.Set(A);
		R.Add(In);
		return R;
	}

	static BigNum Add(BigNum A, BigNum B) {
		BigNum R; R.Set(A);
		R.Add(B);
		return R;
	}

	// b -= a
	void Subtract(BigNumElement In) {
		if (LogicalSize == 0) {
			DebugBreak(); // Can't subtract from an empty BigNum
		} if (In > n[0]) {
			if (LogicalSize > 1) {
				unsigned __int64 i = 0;
				while (!n[++i]) {}
				n[i]--;
				if (n[i] == 0 && i == (LogicalSize - 1)) {
					LogicalSize--;
				}
				for (i = i - 1; i > 0; i--) {
					n[i] = MaxVal;
				}
				n[i] -= In;
			} else {
				DebugBreak(); // Can't subtract from an empty BigNum
			}
		} else {
			n[0] -= In;
		}
	}

	// c = a - b
	static BigNum Subtract(BigNum A, BigNumElement In) {
		BigNum R; R.Set(A);
		R.Subtract(In);
		return R;
	}

	void Subtract(BigNum B) {

		unsigned __int64 BLogicSize = B.LogicalSize;

		if (LogicalSize == 0) {
			DebugBreak(); // Can't subtract from an empty BigNum
		} else if (LogicalSize < BLogicSize) {
			DebugBreak(); // Can't subtract by a larger BigNum
		} else if (LogicalSize == BLogicSize && n[LogicalSize - 1] < B.n[BLogicSize - 1]) {
			DebugBreak(); // Can't subtract by a larger BigNum
		} if (B.MemoryCapacity == 0) {
			DebugBreak(); // PANIC
		}

		for (unsigned __int64 i = (BLogicSize - 1); i != U64Lim; i--) {
			if (n[i] < B.n[i]) {
				unsigned __int64 j = i;
				while (!n[++j]) {}
				n[j]--;
				if (n[j] == 0 && j == (LogicalSize - 1)) {
					LogicalSize--;
				}
				for (j = j - 1; j > i; j--) {
					n[j] = MaxVal;
				}
			}
			n[i] -= B.n[i];
		}
	}


	static BigNum Subtract(BigNum A, BigNum B) {
		BigNum R; R.Set(A);
		R.Subtract(B);
		return R;
	}

	// b *= a
	void Multiply(BigNumElement In) {
		if (In == 1) {
			return;
		} if (MemoryCapacity == 0) {
			DebugBreak();
		} else if (In == 0) {
			for (unsigned __int64 i = 0; i < MemoryCapacity; i++) {
				n[i] = 0;
			} LogicalSize = 0;
			return;
		}

		OverflowDiagnosis ofd = {};
		ofd.j = In;
		unsigned __int64  PrevOverflow = 0;
		unsigned __int64  Limit = LogicalSize;
		unsigned __int64  i = 0;
		for (; i < Limit; i++) {
			ofd.i = n[i];
			ofd.OverflownValue = 0;
			ofd.CalculateOverflow();
			unsigned __int64 NextIndex = i + 1;
			if (ofd.OverflownValue) {
				if (NextIndex == MemoryCapacity) {
					ExpandTo(MemoryCapacity + 10);
				} if (NextIndex == LogicalSize) {
					LogicalSize++;
				}
			}
			n[i] *= In;
			if (PrevOverflow > MaxVal - n[i]) {
				if (NextIndex == MemoryCapacity) {
					ExpandTo(MemoryCapacity + 10);
				} if (NextIndex == LogicalSize) {
					LogicalSize++;
				}
				n[i] += PrevOverflow;
				PrevOverflow = 1 + ofd.OverflownValue;
			} else {
				n[i] += PrevOverflow;
				PrevOverflow = ofd.OverflownValue;
			}
		}
		n[i] += PrevOverflow;
	}

	// c = b*a
	static BigNum Multiply(BigNum B, BigNumElement In) {
		BigNum Result; Result.Set(B);
		Result.MultiplySelf(In);
		return Result;
	}


/*
							111
							*
							011
						=
							011 +
							110 + 
						   1100 
						= 10101
*/

	void MultiplySelf(BigNum B) {
		constexpr unsigned __int64 Bits = 8*GetElementSize();

		BigNum OriginalValue; OriginalValue.Set(*this);
		BigNum Added;
		BigNumElement Val = B.n[0];
		for (unsigned __int64 i = 1; i < (Bits - 1); i++) {
			if ((Val >> i) & 1) {
				Added.Set(OriginalValue);
				Added.LShift(i);
				this->Add(Added);
			}
		}
		for (unsigned __int64 i = 1; i < B.LogicalSize; i++) {
			Val = B.n[i];
			for (unsigned __int64 j = 0; j < (Bits - 1); j++) {
				if ((B.n[i] >> j) & 1) {
					Added.Set(OriginalValue);
					Added.LShift(j + i*Bits);
					this->Add(Added);
				}
			}
		}

		if (!(B.n[0] & 1)) {
			this->Subtract(OriginalValue);
		}
		OriginalValue.Clear();
		Added.Clear();
	}


	// Does not alter the calling BigNum, so it's like a = b / c rather than b /= c.
	static BigNum Divide(BigNum A, BigNumElement In) {
		BigNum Result;
		if (In == 0) {
			DebugBreak(); // Divide by 0
		} else if (In == 1) {
			Result.Set(A);
			return Result;
		} if (A.LogicalSize == 1) {
			BigNumElement Div = A.n[0]/In;
			Result.Add(Div);
			return Result;
		} else if (A.LogicalSize == 0) {
			DebugBreak(); // The BigNum you're trying to divide is empty, probably from a wrongful Clear().
		}

		BigNum Temp(A.LogicalSize);
		Temp.Set(A);

		constexpr unsigned __int64 Bits = 8*GetElementSize();

		struct SubtrahendPowers {
			BigNumElement Upper, Lower;
		}; SubtrahendPowers Subtrahends[Bits - 1] = {};
		unsigned __int64 FirstUpperVar = MaxVal;
		for (unsigned __int64 i = 1; i < Bits; i++) {
			Subtrahends[i - 1].Lower = In << i;
			Subtrahends[i - 1].Upper = In >> (Bits - i);
			if (FirstUpperVar == MaxVal && Subtrahends[i - 1].Upper) {
				FirstUpperVar = i - 1;
			}
		}

		unsigned __int64 Powers[Bits] = {0};
		unsigned __int64 PowersIndex = 0;
		unsigned __int64 Remainder = 0;
		for (unsigned __int64 j = (A.LogicalSize - 1); j != U64Lim;) {
			if (Remainder) {
				unsigned __int64 i = FirstUpperVar;
				for (; i < (Bits - 2); i++) { // optimistically try to find the largest subtrahend
					if (Remainder < Subtrahends[i + 1].Upper) break;
				}
				if (!i && Remainder == Subtrahends[0].Upper && Temp.n[j] < Subtrahends[0].Lower) {
					Remainder--;
					Temp.n[j] -= In;
					i = -1;
				} else if (Temp.n[j] < Subtrahends[i].Lower) { // check to see if it was too large
					if ((Remainder - 1) >= Subtrahends[i].Upper) { // see if we can borrow
						Remainder -= Subtrahends[i].Upper + 1;
						Temp.n[j] -= Subtrahends[i].Lower;
					} else { // if we can't, we must adjust down
						i--;
						if (Temp.n[j] < Subtrahends[i].Lower) { // it could still be that we've overshot
							Remainder -= Subtrahends[i].Upper + 1;
							Temp.n[j] -= Subtrahends[i].Lower;
						} else { // but maybe not
							Remainder -= Subtrahends[i].Upper;
							Temp.n[j] -= Subtrahends[i].Lower;
						}
					}
				} else {
					Remainder -= Subtrahends[i].Upper;
					Temp.n[j] -= Subtrahends[i].Lower;
				}
				Powers[PowersIndex++] = j*Bits + i + 1;

			} else if (Temp.n[j] < In) {
				Remainder = Temp.n[j];
				Temp.n[j] = 0;
				j--;

				if (PowersIndex) {
					if (Powers[0] > (Bits - 1)) {
						BigNumElement ToBeAdded = 0;
						for (unsigned __int64 k = 0; k < PowersIndex; k++) {
							ToBeAdded += (1 << (Powers[k] % (Bits)));
						}
						Result.Add(ToBeAdded);
						Result.LShift(Bits);
					} else {
						for (unsigned __int64 k = 0; k < PowersIndex; k++) {
							Result.Add(1 << (Powers[k]));
						}
					}
				} else {
					if ((j + 2) != Temp.LogicalSize) {
						Result.LShift(Bits);
					}
				} //memset(Powers, 0, Bits*sizeof(unsigned __int64));
				//PowersIndex = 0;
			} else if (Temp.n[j] >= In) {
				if (Temp.n[j] - In < In) {
					Temp.n[j] -= In;
					Powers[PowersIndex++] = j*Bits;
				} else {
					unsigned __int64 i = 0;
					for (; i < (Bits - 1); i++) {
						if (Temp.n[j] < Subtrahends[i + 1].Lower || Subtrahends[i + 1].Upper) break; // We've gone over
					}
					Temp.n[j] -= Subtrahends[i].Lower;
					Powers[PowersIndex++] = j*Bits + i + 1;
				}
			} else if (Temp.n[j] == 0) {
				Result.LShift(Bits);
			}
		}
		Temp.Clear();
		return Result;
	}

	// Does alter the calling BigNum, so it's like b /= c rather than a = b / c
	void Divide(BigNumElement In) {
		BigNum R = Divide(*this, In);
		Set(R);
		R.Clear();
	}

	void LShift(unsigned __int64 By) {
		if (LogicalSize == 0) {
			DebugBreak();
		} else if (By == 0) {
			return;
		}
		__int64 i = LogicalSize - 1;
		unsigned long HighestBit = 0;
		BitScanReverse64(&HighestBit, n[i]);
		unsigned __int64 TotalShift = HighestBit + By;
		constexpr unsigned __int64 Bits = 8*GetElementSize();
		unsigned __int64 AdditionalMemRequired = TotalShift / Bits;
		if (AdditionalMemRequired > MemoryCapacity) {
			ExpandTo(MemoryCapacity + AdditionalMemRequired);
		}
		LogicalSize += AdditionalMemRequired;

		for (; i >= 0; i--) {
			BitScanReverse(&HighestBit, n[i]);
			TotalShift = HighestBit + By;
			if (TotalShift > (Bits - 1)) {
				BigNumElement Overflow = n[i];
				BigNumElement InternalShift = By & (Bits - 1);
				if (InternalShift + HighestBit >= Bits) {
					Overflow >>= ((unsigned __int64) (Bits - By)) & (Bits - 1);
				} else {
					Overflow <<= InternalShift;
				}
				n[i + TotalShift / Bits] += Overflow;
			}
			if (By & (Bits - 1)) {
				n[i] <<= By;
			} else {
				n[i] = 0;
			}
		}
	}

	BigNumElement PopBack() {
		std::string S = ToString();
		Divide(10);
		return (BigNumElement) ((S.at(S.length() - 1)) - '0');
	}
};
