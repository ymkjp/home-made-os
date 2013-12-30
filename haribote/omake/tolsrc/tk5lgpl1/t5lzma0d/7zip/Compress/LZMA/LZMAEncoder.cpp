// LZMA/Encoder.cpp

#include "StdAfx.h"

#include "../../../Common/Defs.h"

#include "LZMAEncoder.h"

// for minimal compressing code size define these:
// #define COMPRESS_MF_BT
// #define COMPRESS_MF_BT4

#if !defined(COMPRESS_MF_BT) && !defined(COMPRESS_MF_PAT) && !defined(COMPRESS_MF_HC)
#define COMPRESS_MF_BT
#define COMPRESS_MF_PAT
#define COMPRESS_MF_HC
#endif

#ifdef COMPRESS_MF_BT
#if !defined(COMPRESS_MF_BT2) && !defined(COMPRESS_MF_BT3) && !defined(COMPRESS_MF_BT4) && !defined(COMPRESS_MF_BT4B)
#define COMPRESS_MF_BT2
#define COMPRESS_MF_BT3
#define COMPRESS_MF_BT4
#define COMPRESS_MF_BT4B
#endif
#ifdef COMPRESS_MF_BT2
#include "../LZ/BinTree/BinTree2.h"
#endif
#ifdef COMPRESS_MF_BT3
#include "../LZ/BinTree/BinTree3.h"
#endif
#ifdef COMPRESS_MF_BT4
#include "../LZ/BinTree/BinTree4.h"
#endif
#ifdef COMPRESS_MF_BT4B
#include "../LZ/BinTree/BinTree4b.h"
#endif
#endif

#ifdef COMPRESS_MF_PAT
#include "../LZ/Patricia/Pat2.h"
#include "../LZ/Patricia/Pat2H.h"
#include "../LZ/Patricia/Pat3H.h"
#include "../LZ/Patricia/Pat4H.h"
#include "../LZ/Patricia/Pat2R.h"
#endif

#ifdef COMPRESS_MF_HC
#include "../LZ/HashChain/HC3.h"
#include "../LZ/HashChain/HC4.h"
#endif

#ifdef COMPRESS_MF_MT
#include "../LZ/MT/MT.h"
#endif

namespace NCompress {
namespace NLZMA {

const int kDefaultDictionaryLogSize = 20;
const UInt32 kNumFastBytesDefault = 0x20;

enum 
{
  kBT2,
  kBT3,
  kBT4,
  kBT4B,
  kPat2,
  kPat2H,
  kPat3H,
  kPat4H,
  kPat2R,
  kHC3,
  kHC4
};

static const wchar_t *kMatchFinderIDs[] = 
{
  L"BT2",
  L"BT3",
  L"BT4",
  L"BT4B",
  L"PAT2",
  L"PAT2H",
  L"PAT3H",
  L"PAT4H",
  L"PAT2R",
  L"HC3",
  L"HC4"
};

Byte g_FastPos[1024];

class CFastPosInit
{
public:
  CFastPosInit() { Init(); }
  void Init()
  {
    const Byte kFastSlots = 20;
    int c = 2;
    g_FastPos[0] = 0;
    g_FastPos[1] = 1;

    for (Byte slotFast = 2; slotFast < kFastSlots; slotFast++)
    {
      UInt32 k = (1 << ((slotFast >> 1) - 1));
      for (UInt32 j = 0; j < k; j++, c++)
        g_FastPos[c] = slotFast;
    }
  }
} g_FastPosInit;


void CLiteralEncoder2::Encode(NRangeCoder::CEncoder *rangeEncoder, 
    bool matchMode, Byte matchByte, Byte symbol)
{
  UInt32 context = 1;
  bool same = true;
  for (int i = 7; i >= 0; i--)
  {
    UInt32 bit = (symbol >> i) & 1;
    UINT state = context;
    if (matchMode && same)
    {
      UInt32 matchBit = (matchByte >> i) & 1;
      state += (1 + matchBit) << 8;
      same = (matchBit == bit);
    }
    _encoders[state].Encode(rangeEncoder, bit);
    context = (context << 1) | bit;
  }
}

UInt32 CLiteralEncoder2::GetPrice(bool matchMode, Byte matchByte, Byte symbol) const
{
  UInt32 price = 0;
  UInt32 context = 1;
  int i = 7;
  if (matchMode)
  {
    for (; i >= 0; i--)
    {
      UInt32 matchBit = (matchByte >> i) & 1;
      UInt32 bit = (symbol >> i) & 1;
      price += _encoders[((1 + matchBit) << 8) + context].GetPrice(bit);
      context = (context << 1) | bit;
      if (matchBit != bit)
      {
        i--;
        break;
      }
    }
  }
  for (; i >= 0; i--)
  {
    UInt32 bit = (symbol >> i) & 1;
    price += _encoders[context].GetPrice(bit);
    context = (context << 1) | bit;
  }
  return price;
};


namespace NLength {

void CEncoder::Init(UInt32 numPosStates)
{
  _choice.Init();
  for (UInt32 posState = 0; posState < numPosStates; posState++)
  {
    _lowCoder[posState].Init();
    _midCoder[posState].Init();
  }
  _choice2.Init();
  _highCoder.Init();
}

void CEncoder::Encode(NRangeCoder::CEncoder *rangeEncoder, UInt32 symbol, UInt32 posState)
{
	if (symbol < kNumLowSymbols) {
		_choice.Encode(rangeEncoder, 0);
		_lowCoder[posState].Encode(rangeEncoder, symbol);
	} else {
		symbol -= kNumLowSymbols;
		_choice.Encode(rangeEncoder, 1);
		if (symbol < kNumMidSymbols) {
			_choice2.Encode(rangeEncoder, 0);
			_midCoder[posState].Encode(rangeEncoder, symbol);
		} else {
			/* 以下KL-01を主張 */
			symbol -= kNumMidSymbols;
			_choice2.Encode(rangeEncoder, 1);
			if (symbol < (1 << kNumHighBits) - 8)
				_highCoder.Encode(rangeEncoder, symbol);
			else {
				int l0, l1;
				symbol -= (1 << kNumHighBits) - 8 - 1; /* 1以上 */
				/* このsymbolを表現するのに何ビット必要だろうか？ */
				for (l0 = 1; symbol >= (1 << l0); l0++);
				/* 答え：l0ビット(l0=1-255) */
				/* このl0を表現するのに何ビット必要だろうか */
				for (l1 = 0; l0 >= (2 << l1); l1++);
				/* 答え：(l1 + 1)ビット(l1=0-7) */
				_highCoder.Encode(rangeEncoder, (1 << kNumHighBits) - 8 + l1);
				if (l1 > 0)
					rangeEncoder->EncodeDirectBits(l0, l1);
				l0--;
				if (l0 > 0)
					rangeEncoder->EncodeDirectBits(symbol, l0);
			}
		}
	}
}

UInt32 CEncoder::GetPrice(UInt32 symbol, UInt32 posState) const
{
  UInt32 price = 0;
  if(symbol < kNumLowSymbols)
  {
    price += _choice.GetPrice(0);
    price += _lowCoder[posState].GetPrice(symbol);
  }
  else
  {
    symbol -= kNumLowSymbols;
    price += _choice.GetPrice(1);
    if(symbol < kNumMidSymbols)
    {
      price += _choice2.GetPrice(0);
      price += _midCoder[posState].GetPrice(symbol);
    }
    else
    {
      price += _choice2.GetPrice(1);
      price += _highCoder.GetPrice(symbol - kNumMidSymbols);
    }
  }
  return price;
}

}
CEncoder::CEncoder():
  _dictionarySize(1 << kDefaultDictionaryLogSize),
  _dictionarySizePrev(UInt32(-1)),
  _numFastBytes(kNumFastBytesDefault),
  _numFastBytesPrev(UInt32(-1)),
  _distTableSize(kDefaultDictionaryLogSize * 2),
  _posStateBits(2),
  _posStateMask(4 - 1),
  _numLiteralPosStateBits(0),
  _numLiteralContextBits(3),
   #ifdef COMPRESS_MF_MT
  _multiThread(false),
   #endif
  _matchFinderIndex(kBT4),
  _writeEndMark(false)
{
  _maxMode = false;
  _fastMode = false;
  _posAlignEncoder.Create(kNumAlignBits);
  for(int i = 0; i < kNumPosModels; i++)
    _posEncoders[i].Create(((kStartPosModelIndex + i) >> 1) - 1);
}

HRESULT CEncoder::Create()
{
  if (!_matchFinder)
  {
    switch(_matchFinderIndex)
    {
      #ifdef COMPRESS_MF_BT
      #ifdef COMPRESS_MF_BT2
      case kBT2:
        _matchFinder = new NBT2::CMatchFinderBinTree;
        break;
      #endif
      #ifdef COMPRESS_MF_BT3
      case kBT3:
        _matchFinder = new NBT3::CMatchFinderBinTree;
        break;
      #endif
      #ifdef COMPRESS_MF_BT4
      case kBT4:
        _matchFinder = new NBT4::CMatchFinderBinTree;
        break;
      #endif
      #ifdef COMPRESS_MF_BT4B
      case kBT4B:
        _matchFinder = new NBT4B::CMatchFinderBinTree;
        break;
      #endif
      #endif
      
      #ifdef COMPRESS_MF_PAT
      case kPat2:
        _matchFinder = new NPat2::CPatricia;
        break;
      case kPat2H:
        _matchFinder = new NPat2H::CPatricia;
        break;
      case kPat3H:
        _matchFinder = new NPat3H::CPatricia;
        break;
      case kPat4H:
        _matchFinder = new NPat4H::CPatricia;
        break;
      case kPat2R:
        _matchFinder = new NPat2R::CPatricia;
        break;
      #endif

      #ifdef COMPRESS_MF_HC
      case kHC3:
        _matchFinder = new NHC3::CMatchFinderHC;
        break;
      case kHC4:
        _matchFinder = new NHC4::CMatchFinderHC;
        break;
      #endif
    }
    #ifdef COMPRESS_MF_MT
    if (_multiThread)
    {
      CMatchFinderMT *mfSpec = new CMatchFinderMT;
      CMyComPtr<IMatchFinder> mf = mfSpec;
      RINOK(mfSpec->SetMatchFinder(_matchFinder));
      _matchFinder.Release();
      _matchFinder = mf;
    }
    #endif
  }
  
  try { _literalEncoder.Create(_numLiteralPosStateBits, _numLiteralContextBits); }
  catch(...) { return E_OUTOFMEMORY;}

  if (_dictionarySize == _dictionarySizePrev && _numFastBytesPrev == _numFastBytes)
    return S_OK;
  RINOK(_matchFinder->Create(_dictionarySize, kNumOpts, _numFastBytes, 
      kMatchMaxLen - _numFastBytes));
  _dictionarySizePrev = _dictionarySize;
  _numFastBytesPrev = _numFastBytes;
  return S_OK;
}

static bool AreStringsEqual(const wchar_t *base, const wchar_t *testString)
{
  while (true)
  {
    wchar_t c = *testString;
    if (c >= 'a' && c <= 'z')
      c -= 0x20;
    if (*base != c)
      return false;
    if (c == 0)
      return true;
    base++;
    testString++;
  }
}

static int FindMatchFinder(const wchar_t *s)
{
  for (int m = 0; m < sizeof(kMatchFinderIDs) / sizeof(kMatchFinderIDs[0]); m++)
    if (AreStringsEqual(kMatchFinderIDs[m], s))
      return m;
  return -1;
}

STDMETHODIMP CEncoder::SetCoderProperties(const PROPID *propIDs, 
    const PROPVARIANT *properties, UInt32 numProperties)
{
  for (UInt32 i = 0; i < numProperties; i++)
  {
    const PROPVARIANT &prop = properties[i];
    switch(propIDs[i])
    {
      case NCoderPropID::kNumFastBytes:
      {
        if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        UInt32 numFastBytes = prop.ulVal;
        if(numFastBytes < 2 || numFastBytes > kMatchMaxLen)
          return E_INVALIDARG;
        _numFastBytes = numFastBytes;
        break;
      }
      case NCoderPropID::kAlgorithm:
      {
        if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        UInt32 maximize = prop.ulVal;
        _fastMode = (maximize == 0); 
        _maxMode = (maximize >= 2);
        break;
      }
      case NCoderPropID::kMatchFinder:
      {
        if (prop.vt != VT_BSTR)
          return E_INVALIDARG;
        int matchFinderIndexPrev = _matchFinderIndex;
        int m = FindMatchFinder(prop.bstrVal);
        if (m < 0)
          return E_INVALIDARG;
        _matchFinderIndex = m;
        if (!_matchFinder && matchFinderIndexPrev != _matchFinderIndex)
        {
          _dictionarySizePrev = UInt32(-1);
          _matchFinder.Release();
        }
        break;
      }
      #ifdef COMPRESS_MF_MT
      case NCoderPropID::kMultiThread:
      {
        if (prop.vt != VT_BOOL)
          return E_INVALIDARG;
        bool newMultiThread = (prop.boolVal == VARIANT_TRUE);
        if (newMultiThread != _multiThread)
        {
          _dictionarySizePrev = UInt32(-1);
          _matchFinder.Release();
        }
        _multiThread = newMultiThread;
        break;
      }
      #endif
      case NCoderPropID::kDictionarySize:
      {
        const int kDicLogSizeMaxCompress = 28;
        if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        UInt32 dictionarySize = prop.ulVal;
        if (dictionarySize < UInt32(1 << kDicLogSizeMin) ||
            dictionarySize > UInt32(1 << kDicLogSizeMaxCompress))
          return E_INVALIDARG;
        _dictionarySize = dictionarySize;
        UInt32 dicLogSize;
        for(dicLogSize = 0; dicLogSize < kDicLogSizeMaxCompress; dicLogSize++)
          if (dictionarySize <= (UInt32(1) << dicLogSize))
            break;
        _distTableSize = dicLogSize * 2;
        break;
      }
      case NCoderPropID::kPosStateBits:
      {
        if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        UInt32 value = prop.ulVal;
        if (value > NLength::kNumPosStatesBitsEncodingMax)
          return E_INVALIDARG;
        _posStateBits = value;
        _posStateMask = (1 << _posStateBits) - 1;
        break;
      }
      case NCoderPropID::kLitPosBits:
      {
        if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        UInt32 value = prop.ulVal;
        if (value > kNumLitPosStatesBitsEncodingMax)
          return E_INVALIDARG;
        _numLiteralPosStateBits = value;
        break;
      }
      case NCoderPropID::kLitContextBits:
      {
        if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        UInt32 value = prop.ulVal;
        if (value > kNumLitContextBitsMax)
          return E_INVALIDARG;
        _numLiteralContextBits = value;
        break;
      }
      case NCoderPropID::kEndMarker:
      {
        if (prop.vt != VT_BOOL)
          return E_INVALIDARG;
        SetWriteEndMarkerMode(prop.boolVal == VARIANT_TRUE);
        break;
      }
      default:
        return E_INVALIDARG;
    }
  }
  return S_OK;
}

STDMETHODIMP CEncoder::WriteCoderProperties(ISequentialOutStream *outStream)
{ 
  Byte firstByte = (_posStateBits * 5 + _numLiteralPosStateBits) * 9 + _numLiteralContextBits;
  RINOK(outStream->Write(&firstByte, sizeof(firstByte), NULL));
  for (int i = 0; i < 4; i++)
  {
    Byte b = Byte(_dictionarySize >> (8 * i));
    RINOK(outStream->Write(&b, sizeof(b), NULL));
  }
  return S_OK;
}

STDMETHODIMP CEncoder::Init(
    ISequentialOutStream *outStream)
{
  CBaseState::Init();

  // RINOK(_matchFinder->Init(inStream));
  _rangeEncoder.Init(outStream);

  int i;
  for(i = 0; i < kNumStates; i++)
  {
    for (UInt32 j = 0; j <= _posStateMask; j++)
    {
      _isMatch[i][j].Init();
      _isRep0Long[i][j].Init();
    }
    _isRep[i].Init();
    _isRepG0[i].Init();
    _isRepG1[i].Init();
    _isRepG2[i].Init();
	_K_isRepExt[i].Init();	/* !!! */
  }

	_K_RepExt.Init(); /* !!! */

  _literalEncoder.Init();

  // _repMatchLenEncoder.Init();
  
  for(i = 0; i < kNumLenToPosStates; i++)
    _posSlotEncoder[i].Init();

  for(i = 0; i < kNumPosModels; i++)
    _posEncoders[i].Init();

  _lenEncoder.Init(1 << _posStateBits);
  _repMatchLenEncoder.Init(1 << _posStateBits);

  _posAlignEncoder.Init();

  _longestMatchWasFound = false;
  _optimumEndIndex = 0;
  _optimumCurrentIndex = 0;
  _additionalOffset = 0;

  return S_OK;
}

void CEncoder::MovePos(UInt32 num)
{
  for (;num > 0; num--)
  {
    _matchFinder->DummyLongestMatch();
    HRESULT result = _matchFinder->MovePos();
    if (result != S_OK)
      throw CMatchFinderException(result);
    _additionalOffset++;
  }
}

UInt32 CEncoder::Backward(UInt32 &backRes, UInt32 cur)
{
  _optimumEndIndex = cur;
  UInt32 posMem = _optimum[cur].PosPrev;
  UInt32 backMem = _optimum[cur].BackPrev;
  do
  {
    if (_optimum[cur].Prev1IsChar)
    {
      _optimum[posMem].MakeAsChar();
      _optimum[posMem].PosPrev = posMem - 1;
      if (_optimum[cur].Prev2)
      {
        _optimum[posMem - 1].Prev1IsChar = false;
        _optimum[posMem - 1].PosPrev = _optimum[cur].PosPrev2;
        _optimum[posMem - 1].BackPrev = _optimum[cur].BackPrev2;
      }
    }
    UInt32 posPrev = posMem;
    UInt32 backCur = backMem;

    backMem = _optimum[posPrev].BackPrev;
    posMem = _optimum[posPrev].PosPrev;

    _optimum[posPrev].BackPrev = backCur;
    _optimum[posPrev].PosPrev = cur;
    cur = posPrev;
  }
  while(cur > 0);
  backRes = _optimum[0].BackPrev;
  _optimumCurrentIndex  = _optimum[0].PosPrev;
  return _optimumCurrentIndex; 
}

/*
inline UInt32 GetMatchLen(const Byte *data, UInt32 back, UInt32 limit)
{  
  back++;
  for(UInt32 i = 0; i < limit && data[i] == data[i - back]; i++);
  return i;
}
*/

UInt32 CEncoder::GetOptimum(UInt32 &backRes, UInt32 position)
{
  if(_optimumEndIndex != _optimumCurrentIndex)
  {
    UInt32 len = _optimum[_optimumCurrentIndex].PosPrev - _optimumCurrentIndex;
    backRes = _optimum[_optimumCurrentIndex].BackPrev;
    _optimumCurrentIndex = _optimum[_optimumCurrentIndex].PosPrev;
    return len;
  }
  _optimumCurrentIndex = 0;
  _optimumEndIndex = 0; // test it;
  
  UInt32 lenMain;
  if (!_longestMatchWasFound)
    lenMain = ReadMatchDistances();
  else
  {
    lenMain = _longestMatchLength;
    _longestMatchWasFound = false;
  }


  UInt32 reps[kNumRepDistances];
  UInt32 repLens[kNumRepDistances];
  UInt32 repMaxIndex = 0;
  int i;
  for(i = 0; i < kNumRepDistances; i++)
  {
    reps[i] = _repDistances[i];
    repLens[i] = _matchFinder->GetMatchLen(0 - 1, reps[i], /* kMatchMaxLen */ _matchFinder->GetNumAvailableBytes());
    if (i == 0 || repLens[i] > repLens[repMaxIndex])
      repMaxIndex = i;
  }
  if(repLens[repMaxIndex] > _numFastBytes)
  {
    backRes = repMaxIndex;
    MovePos(repLens[repMaxIndex] - 1);
    return repLens[repMaxIndex];
  }

  if(lenMain > _numFastBytes)
  {
    UInt32 backMain = (lenMain < _numFastBytes) ? _matchDistances[lenMain] :
        _matchDistances[_numFastBytes];
    backRes = backMain + kNumRepDistances; 
    MovePos(lenMain - 1);
    return lenMain;
  }
  Byte currentByte = _matchFinder->GetIndexByte(0 - 1);

  _optimum[0].State = _state;

  Byte matchByte;
  
  matchByte = _matchFinder->GetIndexByte(0 - _repDistances[0] - 1 - 1);

  UInt32 posState = (position & _posStateMask);

  _optimum[1].Price = _isMatch[_state.Index][posState].GetPrice(0) + 
      _literalEncoder.GetPrice(position, _previousByte, _peviousIsMatch, matchByte, currentByte);
  _optimum[1].MakeAsChar();

  _optimum[1].PosPrev = 0;

  for (i = 0; i < kNumRepDistances; i++)
    _optimum[0].Backs[i] = reps[i];

  UInt32 matchPrice = _isMatch[_state.Index][posState].GetPrice(1);
  UInt32 repMatchPrice = matchPrice + _isRep[_state.Index].GetPrice(1);

  if(matchByte == currentByte)
  {
    UInt32 shortRepPrice = repMatchPrice + GetRepLen1Price(_state, posState);
    if(shortRepPrice < _optimum[1].Price)
    {
      _optimum[1].Price = shortRepPrice;
      _optimum[1].MakeAsShortRep();
    }
  }
  if(lenMain < 2)
  {
    backRes = _optimum[1].BackPrev;
    return 1;
  }

  
  UInt32 normalMatchPrice = matchPrice + 
      _isRep[_state.Index].GetPrice(0);

  if (lenMain <= repLens[repMaxIndex])
    lenMain = 0;

  UInt32 len;
  for(len = 2; len <= lenMain; len++)
  {
    _optimum[len].PosPrev = 0;
    _optimum[len].BackPrev = _matchDistances[len] + kNumRepDistances;
    _optimum[len].Price = normalMatchPrice + 
        GetPosLenPrice(_matchDistances[len], len, posState);
    _optimum[len].Prev1IsChar = false;
  }

  if (lenMain < repLens[repMaxIndex])
    lenMain = repLens[repMaxIndex];

  for (; len <= lenMain; len++)
    _optimum[len].Price = kIfinityPrice;

  for(i = 0; i < kNumRepDistances; i++)
  {
    UINT repLen = repLens[i];
    for(UInt32 lenTest = 2; lenTest <= repLen; lenTest++)
    {
      UInt32 curAndLenPrice = repMatchPrice + GetRepPrice(i, lenTest, _state, posState);
      COptimal &optimum = _optimum[lenTest];
      if (curAndLenPrice < optimum.Price) 
      {
        optimum.Price = curAndLenPrice;
        optimum.PosPrev = 0;
        optimum.BackPrev = i;
        optimum.Prev1IsChar = false;
      }
    }
  }

  UInt32 cur = 0;
  UInt32 lenEnd = lenMain;

  while(true)
  {
    cur++;
    if(cur == lenEnd)  
      return Backward(backRes, cur);
    position++;
    UInt32 posPrev = _optimum[cur].PosPrev;
    CState state;
    if (_optimum[cur].Prev1IsChar)
    {
      posPrev--;
      if (_optimum[cur].Prev2)
      {
        state = _optimum[_optimum[cur].PosPrev2].State;
        if (_optimum[cur].BackPrev2 < kNumRepDistances)
          state.UpdateRep();
        else
          state.UpdateMatch();
      }
      else
        state = _optimum[posPrev].State;
      state.UpdateChar();
    }
    else
      state = _optimum[posPrev].State;
    bool prevWasMatch;
    if (posPrev == cur - 1)
    {
      if (_optimum[cur].IsShortRep())
      {
        prevWasMatch = true;
        state.UpdateShortRep();
      }
      else
      {
        prevWasMatch = false;
        state.UpdateChar();
      }
      /*
      if (_optimum[cur].Prev1IsChar)
        for(int i = 0; i < kNumRepDistances; i++)
          reps[i] = _optimum[posPrev].Backs[i];
      */
    }
    else
    {
      prevWasMatch = true;
      UInt32 pos;
      if (_optimum[cur].Prev1IsChar && _optimum[cur].Prev2)
      {
        posPrev = _optimum[cur].PosPrev2;
        pos = _optimum[cur].BackPrev2;
        state.UpdateRep();
      }
      else
      {
        pos = _optimum[cur].BackPrev;
        if (pos < kNumRepDistances)
          state.UpdateRep();
        else
          state.UpdateMatch();
      }
      if (pos < kNumRepDistances)
      {
        reps[0] = _optimum[posPrev].Backs[pos];
    		UInt32 i;
        for(i = 1; i <= pos; i++)
          reps[i] = _optimum[posPrev].Backs[i - 1];
        for(; i < kNumRepDistances; i++)
          reps[i] = _optimum[posPrev].Backs[i];
      }
      else
      {
        reps[0] = (pos - kNumRepDistances);
        for(UInt32 i = 1; i < kNumRepDistances; i++)
          reps[i] = _optimum[posPrev].Backs[i - 1];
      }
    }
    _optimum[cur].State = state;
    for(UInt32 i = 0; i < kNumRepDistances; i++)
      _optimum[cur].Backs[i] = reps[i];
    UInt32 newLen = ReadMatchDistances();
    if(newLen > _numFastBytes)
    {
      _longestMatchLength = newLen;
      _longestMatchWasFound = true;
      return Backward(backRes, cur);
    }
    UInt32 curPrice = _optimum[cur].Price; 
    // Byte currentByte  = _matchFinder->GetIndexByte(0 - 1);
    // Byte matchByte = _matchFinder->GetIndexByte(0 - reps[0] - 1 - 1);
    const Byte *data = _matchFinder->GetPointerToCurrentPos() - 1;
    Byte currentByte = *data;
    Byte matchByte = data[(size_t)0 - reps[0] - 1];

    UInt32 posState = (position & _posStateMask);

    UInt32 curAnd1Price = curPrice +
        _isMatch[state.Index][posState].GetPrice(0) +
        _literalEncoder.GetPrice(position, data[(size_t)0 - 1], prevWasMatch, matchByte, currentByte);

    COptimal &nextOptimum = _optimum[cur + 1];

    bool nextIsChar = false;
    if (curAnd1Price < nextOptimum.Price) 
    {
      nextOptimum.Price = curAnd1Price;
      nextOptimum.PosPrev = cur;
      nextOptimum.MakeAsChar();
      nextIsChar = true;
    }

    UInt32 matchPrice = curPrice + _isMatch[state.Index][posState].GetPrice(1);
    UInt32 repMatchPrice = matchPrice + _isRep[state.Index].GetPrice(1);
    
    if(matchByte == currentByte &&
        !(nextOptimum.PosPrev < cur && nextOptimum.BackPrev == 0))
    {
      UInt32 shortRepPrice = repMatchPrice + GetRepLen1Price(state, posState);
      if(shortRepPrice <= nextOptimum.Price)
      {
        nextOptimum.Price = shortRepPrice;
        nextOptimum.PosPrev = cur;
        nextOptimum.MakeAsShortRep();
        // nextIsChar = false;
      }
    }
    /*
    if(newLen == 2 && _matchDistances[2] >= kDistLimit2) // test it maybe set 2000 ?
      continue;
    */

    UInt32 numAvailableBytes = _matchFinder->GetNumAvailableBytes() + 1;
    numAvailableBytes = MyMin(kNumOpts - 1 - cur, numAvailableBytes);

    if (numAvailableBytes < 2)
      continue;
    if (numAvailableBytes > _numFastBytes)
      numAvailableBytes = _numFastBytes;
    if (numAvailableBytes >= 3 && !nextIsChar)
    {
      UInt32 backOffset = reps[0] + 1;
      UInt32 temp;
      for (temp = 1; temp < numAvailableBytes; temp++)
        if (data[temp] != data[(size_t)temp - backOffset])
          break;
      UInt32 lenTest2 = temp - 1;
      if (lenTest2 >= 2)
      {
        CState state2 = state;
        state2.UpdateChar();
        UInt32 posStateNext = (position + 1) & _posStateMask;
        UInt32 nextRepMatchPrice = curAnd1Price + 
            _isMatch[state2.Index][posStateNext].GetPrice(1) +
            _isRep[state2.Index].GetPrice(1);
        // for (; lenTest2 >= 2; lenTest2--)
        {
          while(lenEnd < cur + 1 + lenTest2)
            _optimum[++lenEnd].Price = kIfinityPrice;
          UInt32 curAndLenPrice = nextRepMatchPrice + GetRepPrice(
              0, lenTest2, state2, posStateNext);
          COptimal &optimum = _optimum[cur + 1 + lenTest2];
          if (curAndLenPrice < optimum.Price) 
          {
            optimum.Price = curAndLenPrice;
            optimum.PosPrev = cur + 1;
            optimum.BackPrev = 0;
            optimum.Prev1IsChar = true;
            optimum.Prev2 = false;
          }
        }
      }
    }
    for(UInt32 repIndex = 0; repIndex < kNumRepDistances; repIndex++)
    {
      // UInt32 repLen = _matchFinder->GetMatchLen(0 - 1, reps[repIndex], newLen); // test it;
      UInt32 backOffset = reps[repIndex] + 1;
      UInt32 lenTest;
      for (lenTest = 0; lenTest < numAvailableBytes; lenTest++)
        if (data[lenTest] != data[(size_t)lenTest - backOffset])
          break;
      for(; lenTest >= 2; lenTest--)
      {
        while(lenEnd < cur + lenTest)
          _optimum[++lenEnd].Price = kIfinityPrice;
        UInt32 curAndLenPrice = repMatchPrice + GetRepPrice(repIndex, lenTest, state, posState);
        COptimal &optimum = _optimum[cur + lenTest];
        if (curAndLenPrice < optimum.Price) 
        {
          optimum.Price = curAndLenPrice;
          optimum.PosPrev = cur;
          optimum.BackPrev = repIndex;
          optimum.Prev1IsChar = false;
        }

        /*
        if (_maxMode)
        {
          UInt32 temp;
          for (temp = lenTest + 1; temp < numAvailableBytes; temp++)
            if (data[temp] != data[(size_t)temp - backOffset])
              break;
          UInt32 lenTest2 = temp - (lenTest + 1);
          if (lenTest2 >= 2)
          {
            CState state2 = state;
            state2.UpdateRep();
            UInt32 posStateNext = (position + lenTest) & _posStateMask;
            UInt32 curAndLenCharPrice = curAndLenPrice + 
                _isMatch[state2.Index][posStateNext].GetPrice(0) +
                _literalEncoder.GetPrice(position + lenTest, data[(size_t)lenTest - 1], 
                true, data[(size_t)lenTest - backOffset], data[lenTest]);
            state2.UpdateChar();
            posStateNext = (position + lenTest + 1) & _posStateMask;
            UInt32 nextMatchPrice = curAndLenCharPrice + _isMatch[state2.Index][posStateNext].GetPrice(1);
            UInt32 nextRepMatchPrice = nextMatchPrice + _isRep[state2.Index].GetPrice(1);
            
            // for(; lenTest2 >= 2; lenTest2--)
            {
              UInt32 offset = lenTest + 1 + lenTest2;
              while(lenEnd < cur + offset)
                _optimum[++lenEnd].Price = kIfinityPrice;
              UInt32 curAndLenPrice = nextRepMatchPrice + GetRepPrice(
                  0, lenTest2, state2, posStateNext);
              COptimal &optimum = _optimum[cur + offset];
              if (curAndLenPrice < optimum.Price) 
              {
                optimum.Price = curAndLenPrice;
                optimum.PosPrev = cur + lenTest + 1;
                optimum.BackPrev = 0;
                optimum.Prev1IsChar = true;
                optimum.Prev2 = true;
                optimum.PosPrev2 = cur;
                optimum.BackPrev2 = repIndex;
              }
            }
          }
        }
        */
      }
    }
    
    //    for(UInt32 lenTest = 2; lenTest <= newLen; lenTest++)
    if (newLen > numAvailableBytes)
      newLen = numAvailableBytes;
    if (newLen >= 2)
    {
      if (newLen == 2 && _matchDistances[2] >= 0x80)
        continue;
      UInt32 normalMatchPrice = matchPrice + 
        _isRep[state.Index].GetPrice(0);
      while(lenEnd < cur + newLen)
        _optimum[++lenEnd].Price = kIfinityPrice;

      for(UInt32 lenTest = newLen; lenTest >= 2; lenTest--)
      {
        UInt32 curBack = _matchDistances[lenTest];
        UInt32 curAndLenPrice = normalMatchPrice + GetPosLenPrice(curBack, lenTest, posState);
        COptimal &optimum = _optimum[cur + lenTest];
        if (curAndLenPrice < optimum.Price) 
        {
          optimum.Price = curAndLenPrice;
          optimum.PosPrev = cur;
          optimum.BackPrev = curBack + kNumRepDistances;
          optimum.Prev1IsChar = false;
        }

        if (_maxMode)
        {
          UInt32 backOffset = curBack + 1;
          UInt32 temp;
          for (temp = lenTest + 1; temp < numAvailableBytes; temp++)
            if (data[temp] != data[(size_t)temp - backOffset])
              break;
          UInt32 lenTest2 = temp - (lenTest + 1);
          if (lenTest2 >= 2)
          {
            CState state2 = state;
            state2.UpdateMatch();
            UInt32 posStateNext = (position + lenTest) & _posStateMask;
            UInt32 curAndLenCharPrice = curAndLenPrice + 
                _isMatch[state2.Index][posStateNext].GetPrice(0) +
                _literalEncoder.GetPrice(position + lenTest, data[(size_t)lenTest - 1], 
                true, data[(size_t)lenTest - backOffset], data[lenTest]);
            state2.UpdateChar();
            posStateNext = (position + lenTest + 1) & _posStateMask;
            UInt32 nextMatchPrice = curAndLenCharPrice + _isMatch[state2.Index][posStateNext].GetPrice(1);
            UInt32 nextRepMatchPrice = nextMatchPrice + _isRep[state2.Index].GetPrice(1);
            
            // for(; lenTest2 >= 2; lenTest2--)
            {
              UInt32 offset = lenTest + 1 + lenTest2;
              while(lenEnd < cur + offset)
                _optimum[++lenEnd].Price = kIfinityPrice;
              UInt32 curAndLenPrice = nextRepMatchPrice + GetRepPrice(
                  0, lenTest2, state2, posStateNext);
              COptimal &optimum = _optimum[cur + offset];
              if (curAndLenPrice < optimum.Price) 
              {
                optimum.Price = curAndLenPrice;
                optimum.PosPrev = cur + lenTest + 1;
                optimum.BackPrev = 0;
                optimum.Prev1IsChar = true;
                optimum.Prev2 = true;
                optimum.PosPrev2 = cur;
                optimum.BackPrev2 = curBack + kNumRepDistances;
              }
            }
          }
        }
      }
    }
  }
}

static inline bool ChangePair(UInt32 smallDist, UInt32 bigDist)
{
  const int kDif = 7;
  return (smallDist < (UInt32(1) << (32-kDif)) && bigDist >= (smallDist << kDif));
}


UInt32 CEncoder::GetOptimumFast(UInt32 &backRes, UInt32 position)
{
  UInt32 lenMain;
  if (!_longestMatchWasFound)
    lenMain = ReadMatchDistances();
  else
  {
    lenMain = _longestMatchLength;
    _longestMatchWasFound = false;
  }
  UInt32 repLens[kNumRepDistances];
  UInt32 repMaxIndex = 0;
  for(int i = 0; i < kNumRepDistances; i++)
  {
    repLens[i] = _matchFinder->GetMatchLen(0 - 1, _repDistances[i], /* kMatchMaxLen */ _matchFinder->GetNumAvailableBytes());
    if (i == 0 || repLens[i] > repLens[repMaxIndex])
      repMaxIndex = i;
  }
  if(repLens[repMaxIndex] >= _numFastBytes)
  {
    backRes = repMaxIndex;
    MovePos(repLens[repMaxIndex] - 1);
    return repLens[repMaxIndex];
  }
  if(lenMain >= _numFastBytes)
  {
    backRes = _matchDistances[_numFastBytes] + kNumRepDistances; 
    MovePos(lenMain - 1);
    return lenMain;
  }
  while (lenMain > 2)
  {
    if (!ChangePair(_matchDistances[lenMain - 1], _matchDistances[lenMain]))
      break;
    lenMain--;
  }
  if (lenMain == 2 && _matchDistances[2] >= 0x80)
    lenMain = 1;

  UInt32 backMain = _matchDistances[lenMain];
  if (repLens[repMaxIndex] >= 2)
  {
    if (repLens[repMaxIndex] + 1 >= lenMain || 
        repLens[repMaxIndex] + 2 >= lenMain && (backMain > (1<<12)))
    {
      backRes = repMaxIndex;
      MovePos(repLens[repMaxIndex] - 1);
      return repLens[repMaxIndex];
    }
  }
  

  if (lenMain >= 2)
  {
    _longestMatchLength = ReadMatchDistances();
    if (_longestMatchLength >= 2 &&
      (
        (_longestMatchLength >= lenMain && 
          _matchDistances[lenMain] < backMain) || 
        _longestMatchLength == lenMain + 1 && 
          !ChangePair(backMain, _matchDistances[_longestMatchLength]) ||
        _longestMatchLength > lenMain + 1 ||
        _longestMatchLength + 1 >= lenMain && 
          ChangePair(_matchDistances[lenMain - 1], backMain)
      )
      )
    {
      _longestMatchWasFound = true;
      backRes = UInt32(-1);
      return 1;
    }
    for(int i = 0; i < kNumRepDistances; i++)
    {
      UInt32 repLen = _matchFinder->GetMatchLen(0 - 1, _repDistances[i], /* kMatchMaxLen */ _matchFinder->GetNumAvailableBytes());
      if (repLen >= 2 && repLen + 1 >= lenMain)
      {
        _longestMatchWasFound = true;
        backRes = UInt32(-1);
        return 1;
      }
    }
    backRes = backMain + kNumRepDistances; 
    MovePos(lenMain - 2);
    return lenMain;
  }
  backRes = UInt32(-1);
  return 1;
}

STDMETHODIMP CEncoder::InitMatchFinder(IMatchFinder *matchFinder)
{
  _matchFinder = matchFinder;
  return S_OK;
}

HRESULT CEncoder::Flush()
{
  _rangeEncoder.FlushData();
  return _rangeEncoder.FlushStream();
}

void CEncoder::WriteEndMarker(UInt32 posState)
{
  // This function for writing End Mark for stream version of LZMA. 
  // In current version this feature is not used.
  if (!_writeEndMark)
    return;

  _isMatch[_state.Index][posState].Encode(&_rangeEncoder, 1);
  _isRep[_state.Index].Encode(&_rangeEncoder, 0);
  _state.UpdateMatch();
  UInt32 len = kMatchMinLen; // kMatchMaxLen;
  _lenEncoder.Encode(&_rangeEncoder, len - kMatchMinLen, posState);
  UInt32 posSlot = (1 << kNumPosSlotBits)  - 1;
  UInt32 lenToPosState = GetLenToPosState(len);
  _posSlotEncoder[lenToPosState].Encode(&_rangeEncoder, posSlot);
  UInt32 footerBits = 30;
  UInt32 posReduced = (UInt32(1) << footerBits) - 1;
  _rangeEncoder.EncodeDirectBits(posReduced >> kNumAlignBits, footerBits - kNumAlignBits);
  _posAlignEncoder.Encode(&_rangeEncoder, posReduced & kAlignMask);
}

HRESULT CEncoder::CodeReal(ISequentialInStream *inStream,
      ISequentialOutStream *outStream, 
      const UInt64 *inSize, const UInt64 *outSize,
      ICompressProgressInfo *progress)
{
  _needReleaseMFStream = false;
  CCoderReleaser coderReleaser(this);
  RINOK(SetStreams(inStream, outStream, inSize, outSize));
  while(true)
  {
    UInt64 processedInSize;
    UInt64 processedOutSize;
    Int32 finished;
    RINOK(CodeOneBlock(&processedInSize, &processedOutSize, &finished));
    if (finished != 0)
      return S_OK;
    if (progress != 0)
    {
      RINOK(progress->SetRatioInfo(&processedInSize, &processedOutSize));
    }
  }
}

HRESULT CEncoder::SetStreams(ISequentialInStream *inStream,
      ISequentialOutStream *outStream, 
      const UInt64 *inSize, const UInt64 *outSize)
{
  _inStream = inStream;
  _finished = false;
  RINOK(Create());
  RINOK(Init(outStream));
  
  // CCoderReleaser releaser(this);

  /*
  if (_matchFinder->GetNumAvailableBytes() == 0)
    return Flush();
  */

  if (!_fastMode)
  {
    FillPosSlotPrices();
    FillDistancesPrices();
    FillAlignPrices();
  }

  _lenEncoder.SetTableSize(_numFastBytes);
  _lenEncoder.UpdateTables(1 << _posStateBits);
  _repMatchLenEncoder.SetTableSize(_numFastBytes);
  _repMatchLenEncoder.UpdateTables(1 << _posStateBits);

  lastPosSlotFillingPos = 0;
  nowPos64 = 0;
  return S_OK;
}

HRESULT CEncoder::CodeOneBlock(UInt64 *inSize, UInt64 *outSize, Int32 *finished)
{
	char _K_erd = 0;
  if (_inStream != 0)
  {
    RINOK(_matchFinder->Init(_inStream));
    _needReleaseMFStream = true;
    _inStream = 0;
  }


  *finished = 1;
  if (_finished)
    return S_OK;
  _finished = true;


  UInt64 progressPosValuePrev = nowPos64;
  if (nowPos64 == 0)
  {
    if (_matchFinder->GetNumAvailableBytes() == 0)
    {
      ReleaseStreams();
      WriteEndMarker(UInt32(nowPos64) & _posStateMask);
      return Flush();
    }
    ReadMatchDistances();
    UInt32 posState = UInt32(nowPos64) & _posStateMask;
    _isMatch[_state.Index][posState].Encode(&_rangeEncoder, 0);
    _state.UpdateChar();
    Byte curByte = _matchFinder->GetIndexByte(0 - _additionalOffset);
    _literalEncoder.Encode(&_rangeEncoder, UInt32(nowPos64), _previousByte, 
      false, 0, curByte);
    _previousByte = curByte;
    _additionalOffset--;
    nowPos64++;
  }
  if (_matchFinder->GetNumAvailableBytes() == 0)
  {
    ReleaseStreams();
    WriteEndMarker(UInt32(nowPos64) & _posStateMask);
    return Flush();
  }
  while(true)
  {
    UInt32 pos;
    UInt32 posState = UInt32(nowPos64) & _posStateMask;

    UInt32 len;
    if (_fastMode)
      len = GetOptimumFast(pos, UInt32(nowPos64));
    else
      len = GetOptimum(pos, UInt32(nowPos64));

    if(len == 1 && pos == (-1))
    {
      _isMatch[_state.Index][posState].Encode(&_rangeEncoder, 0);
      _state.UpdateChar();
      Byte matchByte;
      if(_peviousIsMatch)
        matchByte = _matchFinder->GetIndexByte(0 - _repDistances[0] - 1 - _additionalOffset);
      Byte curByte = _matchFinder->GetIndexByte(0 - _additionalOffset);
      _literalEncoder.Encode(&_rangeEncoder, UInt32(nowPos64), _previousByte, _peviousIsMatch, 
          matchByte, curByte);
      _previousByte = curByte;
      _peviousIsMatch = false;
    }
    else
    {
      _peviousIsMatch = true;
      _isMatch[_state.Index][posState].Encode(&_rangeEncoder, 1);
      if(pos < kNumRepDistances)
      {
        _isRep[_state.Index].Encode(&_rangeEncoder, 1);
        if(pos == 0)
        {
          _isRepG0[_state.Index].Encode(&_rangeEncoder, 0);
          if(len == 1)
            _isRep0Long[_state.Index][posState].Encode(&_rangeEncoder, 0);
          else
            _isRep0Long[_state.Index][posState].Encode(&_rangeEncoder, 1);
        }
        else
        {
          _isRepG0[_state.Index].Encode(&_rangeEncoder, 1);
          if (pos == 1)
            _isRepG1[_state.Index].Encode(&_rangeEncoder, 0);
          else
          {
            _isRepG1[_state.Index].Encode(&_rangeEncoder, 1);
            _isRepG2[_state.Index].Encode(&_rangeEncoder, pos - 2);
			if (pos == 3 && _K_erd != 0)
            	_K_isRepExt[_state.Index].Encode(&_rangeEncoder, 0); /* !!! */
          }
        }
        if (len == 1)
          _state.UpdateShortRep();
        else
        {
          _repMatchLenEncoder.Encode(&_rangeEncoder, len - kMatchMinLen, posState);
          _state.UpdateRep();
        }


        UInt32 distance = _repDistances[pos];
        if (pos != 0)
        {
          for(UInt32 i = pos; i >= 1; i--)
            _repDistances[i] = _repDistances[i - 1];
          _repDistances[0] = distance;
        }
      }
      else
      {
        _isRep[_state.Index].Encode(&_rangeEncoder, 0);
        _state.UpdateMatch();
        _lenEncoder.Encode(&_rangeEncoder, len - kMatchMinLen, posState);
        pos -= kNumRepDistances;
        UInt32 posSlot = GetPosSlot(pos);
        UInt32 lenToPosState = GetLenToPosState(len);
        _posSlotEncoder[lenToPosState].Encode(&_rangeEncoder, posSlot);
        
        if (posSlot >= kStartPosModelIndex)
        {
          UInt32 footerBits = ((posSlot >> 1) - 1);
          UInt32 posReduced = pos - ((2 | (posSlot & 1)) << footerBits);

          if (posSlot < kEndPosModelIndex)
            _posEncoders[posSlot - kStartPosModelIndex].Encode(&_rangeEncoder, posReduced);
          else
          {
            _rangeEncoder.EncodeDirectBits(posReduced >> kNumAlignBits, footerBits - kNumAlignBits);
            _posAlignEncoder.Encode(&_rangeEncoder, posReduced & kAlignMask);
            if (!_fastMode)
              if (--_alignPriceCount == 0)
                FillAlignPrices();
          }
        }
        UInt32 distance = pos;
        for(UInt32 i = kNumRepDistances - 1; i >= 1; i--)
          _repDistances[i] = _repDistances[i - 1];
        _repDistances[0] = distance;
      }
      _previousByte = _matchFinder->GetIndexByte(len - 1 - _additionalOffset);
    }
    _additionalOffset -= len;
    nowPos64 += len;
    if (!_fastMode)
      if (nowPos64 - lastPosSlotFillingPos >= (1 << 9))
      {
        FillPosSlotPrices();
        FillDistancesPrices();
        lastPosSlotFillingPos = nowPos64;
      }
    if (_additionalOffset == 0)
    {
      *inSize = nowPos64;
      *outSize = _rangeEncoder.GetProcessedSize();
      if (_matchFinder->GetNumAvailableBytes() == 0)
      {
        ReleaseStreams();
        WriteEndMarker(UInt32(nowPos64) & _posStateMask);
        return Flush();
      }
      if (nowPos64 - progressPosValuePrev >= (1 << 12))
      {
        _finished = false;
        *finished = 0;
        return S_OK;
      }
    }
  }
}

STDMETHODIMP CEncoder::Code(ISequentialInStream *inStream,
    ISequentialOutStream *outStream, const UInt64 *inSize, const UInt64 *outSize,
    ICompressProgressInfo *progress)
{
  try { return CodeReal(inStream, outStream, inSize, outSize, progress); }
  catch(CMatchFinderException &e) { return e.ErrorCode; }
  catch(const COutBufferException &e) { return e.ErrorCode; }
  catch(...) { return E_FAIL; }
}
  
void CEncoder::FillPosSlotPrices()
{
  for (int lenToPosState = 0; lenToPosState < kNumLenToPosStates; lenToPosState++)
  {
	  UInt32 posSlot;
    for (posSlot = 0; posSlot < kEndPosModelIndex && posSlot < _distTableSize; posSlot++)
      _posSlotPrices[lenToPosState][posSlot] = _posSlotEncoder[lenToPosState].GetPrice(posSlot);
    for (; posSlot < _distTableSize; posSlot++)
      _posSlotPrices[lenToPosState][posSlot] = _posSlotEncoder[lenToPosState].GetPrice(posSlot) + 
      ((((posSlot >> 1) - 1) - kNumAlignBits) << NRangeCoder::kNumBitPriceShiftBits);
  }
}

void CEncoder::FillDistancesPrices()
{
  for (int lenToPosState = 0; lenToPosState < kNumLenToPosStates; lenToPosState++)
  {
	  UInt32 i;
    for (i = 0; i < kStartPosModelIndex; i++)
      _distancesPrices[lenToPosState][i] = _posSlotPrices[lenToPosState][i];
    for (; i < kNumFullDistances; i++)
    { 
      UInt32 posSlot = GetPosSlot(i);
      _distancesPrices[lenToPosState][i] = _posSlotPrices[lenToPosState][posSlot] +
          _posEncoders[posSlot - kStartPosModelIndex].GetPrice(i - 
          ((2 | (posSlot & 1)) << (((posSlot >> 1) - 1))));
    }
  }
}

void CEncoder::FillAlignPrices()
{
  for (int i = 0; i < kAlignTableSize; i++)
    _alignPrices[i] = _posAlignEncoder.GetPrice(i);
  _alignPriceCount = kAlignTableSize;
}

}}
