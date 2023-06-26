#pragma once
struct UTexture2D;
struct UMaterial;

template<class T>
class TArray
{
	friend class FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline int Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline const T& operator[](int i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Num();
	}

private:
	T* Data;
	int Count;
	int Max;
};

struct FNameEntryId
{
	unsigned int Value;
};

struct FName
{
	FNameEntryId ComparisonIndex;
	unsigned int Number;
};

struct $1324AFBE8703442A073325624A2474B6
{
	char B;
	char G;
	char R;
	char A;
};

union $B7B3F0AA4E60EE740ADAA9CB41035D76
{
	$1324AFBE8703442A073325624A2474B6 __s0;
	unsigned int AlignmentDummy;
};

/* 14627 */
struct FColor
{
	$B7B3F0AA4E60EE740ADAA9CB41035D76 ___u0;
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		const auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};
