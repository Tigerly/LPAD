#define DIGEST_SIZE 32
#include <string.h>
unsigned char m_state[200] __attribute__ ((aligned(8)));
unsigned int m_counter = 0;
typedef unsigned long long word64;
typedef unsigned int word32;

inline unsigned long long rotlFixed(unsigned long long x, unsigned int y)
{
  return y?(word64)((x<<y) | (x>>(64-y))) : x;
} 

void xorbuf(unsigned char *buf, const unsigned char *mask, unsigned int count)
{

  unsigned int i;
  for (i=0; i<count/8; i++)
    ((word64*)buf)[i] ^= ((word64*)mask)[i];
  count -= 8*i;
  if (!count)
    return;
  buf += 8*i;
  mask += 8*i;

  for (i=0; i<count; i++)
    buf[i] ^= mask[i];
}

static const word64 KeccakF_RoundConstants[24] = 
{
  0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
  0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
  0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
  0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
  0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
  0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL, 
  0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
  0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static void KeccakF1600(unsigned char *state)
{
  {
    unsigned int round=0;
    word64 Aba, Abe, Abi, Abo, Abu;
    word64 Aga, Age, Agi, Ago, Agu;
    word64 Aka, Ake, Aki, Ako, Aku;
    word64 Ama, Ame, Ami, Amo, Amu;
    word64 Asa, Ase, Asi, Aso, Asu;
    word64 BCa, BCe, BCi, BCo, BCu;
    word64 Da, De, Di, Do, Du;
    word64 Eba, Ebe, Ebi, Ebo, Ebu;
    word64 Ega, Ege, Egi, Ego, Egu;
    word64 Eka, Eke, Eki, Eko, Eku;
    word64 Ema, Eme, Emi, Emo, Emu;
    word64 Esa, Ese, Esi, Eso, Esu;

    //copyFromState(A, state)
    //typedef BlockGetAndPut<word64, LittleEndian, true, true> Block;
    //Block::Get(state)(Aba)(Abe)(Abi)(Abo)(Abu)(Aga)(Age)(Agi)(Ago)(Agu)(Aka)(Ake)(Aki)(Ako)(Aku)(Ama)(Ame)(Ami)(Amo)(Amu)(Asa)(Ase)(Asi)(Aso)(Asu);

    memcpy(&Aba,m_state,8);
    memcpy(&Abe,m_state+8,8);
    memcpy(&Abi,m_state+16,8);
    memcpy(&Abo,m_state+24,8);
    memcpy(&Abu,m_state+32,8);
    memcpy(&Aga,m_state+40,8);
    memcpy(&Age,m_state+48,8);
    memcpy(&Agi,m_state+56,8);
    memcpy(&Ago,m_state+64,8);
    memcpy(&Agu,m_state+72,8);
    memcpy(&Aka,m_state+80,8);
    memcpy(&Ake,m_state+88,8);
    memcpy(&Aki,m_state+96,8);
    memcpy(&Ako,m_state+104,8);
    memcpy(&Aku,m_state+112,8);
    memcpy(&Ama,m_state+120,8);
    memcpy(&Ame,m_state+128,8);
    memcpy(&Ami,m_state+136,8);
    memcpy(&Amo,m_state+144,8);
    memcpy(&Amu,m_state+152,8);
    memcpy(&Asa,m_state+160,8);
    memcpy(&Ase,m_state+168,8);
    memcpy(&Asi,m_state+176,8);
    memcpy(&Aso,m_state+184,8);
    memcpy(&Asu,m_state+192,8);
    for(round = 0; round < 24; round += 2 )
    {
      //    prepareTheta
      BCa = Aba^Aga^Aka^Ama^Asa;
      BCe = Abe^Age^Ake^Ame^Ase;
      BCi = Abi^Agi^Aki^Ami^Asi;
      BCo = Abo^Ago^Ako^Amo^Aso;
      BCu = Abu^Agu^Aku^Amu^Asu;

      //thetaRhoPiChiIotaPrepareTheta(round  , A, E)
      Da = BCu^rotlFixed(BCe, 1);
      De = BCa^rotlFixed(BCi, 1);
      Di = BCe^rotlFixed(BCo, 1);
      Do = BCi^rotlFixed(BCu, 1);
      Du = BCo^rotlFixed(BCa, 1);

      Aba ^= Da;
      BCa = Aba;
      Age ^= De;
      BCe = rotlFixed(Age, 44);
      Aki ^= Di;
      BCi = rotlFixed(Aki, 43);
      Amo ^= Do;
      BCo = rotlFixed(Amo, 21);
      Asu ^= Du;
      BCu = rotlFixed(Asu, 14);
      Eba =   BCa ^((~BCe)&  BCi );
      Eba ^= (word64)KeccakF_RoundConstants[round];
      Ebe =   BCe ^((~BCi)&  BCo );
      Ebi =   BCi ^((~BCo)&  BCu );
      Ebo =   BCo ^((~BCu)&  BCa );
      Ebu =   BCu ^((~BCa)&  BCe );

      Abo ^= Do;
      BCa = rotlFixed(Abo, 28);
      Agu ^= Du;
      BCe = rotlFixed(Agu, 20);
      Aka ^= Da;
      BCi = rotlFixed(Aka,  3);
      Ame ^= De;
      BCo = rotlFixed(Ame, 45);
      Asi ^= Di;
      BCu = rotlFixed(Asi, 61);
      Ega =   BCa ^((~BCe)&  BCi );
      Ege =   BCe ^((~BCi)&  BCo );
      Egi =   BCi ^((~BCo)&  BCu );
      Ego =   BCo ^((~BCu)&  BCa );
      Egu =   BCu ^((~BCa)&  BCe );

      Abe ^= De;
      BCa = rotlFixed(Abe,  1);
      Agi ^= Di;
      BCe = rotlFixed(Agi,  6);
      Ako ^= Do;
      BCi = rotlFixed(Ako, 25);
      Amu ^= Du;
      BCo = rotlFixed(Amu,  8);
      Asa ^= Da;
      BCu = rotlFixed(Asa, 18);
      Eka =   BCa ^((~BCe)&  BCi );
      Eke =   BCe ^((~BCi)&  BCo );
      Eki =   BCi ^((~BCo)&  BCu );
      Eko =   BCo ^((~BCu)&  BCa );
      Eku =   BCu ^((~BCa)&  BCe );

      Abu ^= Du;
      BCa = rotlFixed(Abu, 27);
      Aga ^= Da;
      BCe = rotlFixed(Aga, 36);
      Ake ^= De;
      BCi = rotlFixed(Ake, 10);
      Ami ^= Di;
      BCo = rotlFixed(Ami, 15);
      Aso ^= Do;
      BCu = rotlFixed(Aso, 56);
      Ema =   BCa ^((~BCe)&  BCi );
      Eme =   BCe ^((~BCi)&  BCo );
      Emi =   BCi ^((~BCo)&  BCu );
      Emo =   BCo ^((~BCu)&  BCa );
      Emu =   BCu ^((~BCa)&  BCe );

      Abi ^= Di;
      BCa = rotlFixed(Abi, 62);
      Ago ^= Do;
      BCe = rotlFixed(Ago, 55);
      Aku ^= Du;
      BCi = rotlFixed(Aku, 39);
      Ama ^= Da;
      BCo = rotlFixed(Ama, 41);
      Ase ^= De;
      BCu = rotlFixed(Ase,  2);
      Esa =   BCa ^((~BCe)&  BCi );
      Ese =   BCe ^((~BCi)&  BCo );
      Esi =   BCi ^((~BCo)&  BCu );
      Eso =   BCo ^((~BCu)&  BCa );
      Esu =   BCu ^((~BCa)&  BCe );

      //    prepareTheta
      BCa = Eba^Ega^Eka^Ema^Esa;
      BCe = Ebe^Ege^Eke^Eme^Ese;
      BCi = Ebi^Egi^Eki^Emi^Esi;
      BCo = Ebo^Ego^Eko^Emo^Eso;
      BCu = Ebu^Egu^Eku^Emu^Esu;

      //thetaRhoPiChiIotaPrepareTheta(round+1, E, A)
      Da = BCu^rotlFixed(BCe, 1);
      De = BCa^rotlFixed(BCi, 1);
      Di = BCe^rotlFixed(BCo, 1);
      Do = BCi^rotlFixed(BCu, 1);
      Du = BCo^rotlFixed(BCa, 1);

      Eba ^= Da;
      BCa = Eba;
      Ege ^= De;
      BCe = rotlFixed(Ege, 44);
      Eki ^= Di;
      BCi = rotlFixed(Eki, 43);
      Emo ^= Do;
      BCo = rotlFixed(Emo, 21);
      Esu ^= Du;
      BCu = rotlFixed(Esu, 14);
      Aba =   BCa ^((~BCe)&  BCi );
      Aba ^= (word64)KeccakF_RoundConstants[round+1];
      Abe =   BCe ^((~BCi)&  BCo );
      Abi =   BCi ^((~BCo)&  BCu );
      Abo =   BCo ^((~BCu)&  BCa );
      Abu =   BCu ^((~BCa)&  BCe );

      Ebo ^= Do;
      BCa = rotlFixed(Ebo, 28);
      Egu ^= Du;
      BCe = rotlFixed(Egu, 20);
      Eka ^= Da;
      BCi = rotlFixed(Eka, 3);
      Eme ^= De;
      BCo = rotlFixed(Eme, 45);
      Esi ^= Di;
      BCu = rotlFixed(Esi, 61);
      Aga =   BCa ^((~BCe)&  BCi );
      Age =   BCe ^((~BCi)&  BCo );
      Agi =   BCi ^((~BCo)&  BCu );
      Ago =   BCo ^((~BCu)&  BCa );
      Agu =   BCu ^((~BCa)&  BCe );

      Ebe ^= De;
      BCa = rotlFixed(Ebe, 1);
      Egi ^= Di;
      BCe = rotlFixed(Egi, 6);
      Eko ^= Do;
      BCi = rotlFixed(Eko, 25);
      Emu ^= Du;
      BCo = rotlFixed(Emu, 8);
      Esa ^= Da;
      BCu = rotlFixed(Esa, 18);
      Aka =   BCa ^((~BCe)&  BCi );
      Ake =   BCe ^((~BCi)&  BCo );
      Aki =   BCi ^((~BCo)&  BCu );
      Ako =   BCo ^((~BCu)&  BCa );
      Aku =   BCu ^((~BCa)&  BCe );

      Ebu ^= Du;
      BCa = rotlFixed(Ebu, 27);
      Ega ^= Da;
      BCe = rotlFixed(Ega, 36);
      Eke ^= De;
      BCi = rotlFixed(Eke, 10);
      Emi ^= Di;
      BCo = rotlFixed(Emi, 15);
      Eso ^= Do;
      BCu = rotlFixed(Eso, 56);
      Ama =   BCa ^((~BCe)&  BCi );
      Ame =   BCe ^((~BCi)&  BCo );
      Ami =   BCi ^((~BCo)&  BCu );
      Amo =   BCo ^((~BCu)&  BCa );
      Amu =   BCu ^((~BCa)&  BCe );

      Ebi ^= Di;
      BCa = rotlFixed(Ebi, 62);
      Ego ^= Do;
      BCe = rotlFixed(Ego, 55);
      Eku ^= Du;
      BCi = rotlFixed(Eku, 39);
      Ema ^= Da;
      BCo = rotlFixed(Ema, 41);
      Ese ^= De;
      BCu = rotlFixed(Ese, 2);
      Asa =   BCa ^((~BCe)&  BCi );
      Ase =   BCe ^((~BCi)&  BCo );
      Asi =   BCi ^((~BCo)&  BCu );
      Aso =   BCo ^((~BCu)&  BCa );
      Asu =   BCu ^((~BCa)&  BCe );
    }

    memcpy(m_state,&Aba,8);
    memcpy(m_state+8,&Abe,8);
    memcpy(m_state+16,&Abi,8);
    memcpy(m_state+24,&Abo,8);
    memcpy(m_state+32,&Abu,8);
    memcpy(m_state+40,&Aga,8);
    memcpy(m_state+48,&Age,8);
    memcpy(m_state+56,&Agi,8);
    memcpy(m_state+64,&Ago,8);
    memcpy(m_state+72,&Agu,8);
    memcpy(m_state+80,&Aka,8);
    memcpy(m_state+88,&Ake,8);
    memcpy(m_state+96,&Aki,8);
    memcpy(m_state+104,&Ako,8);
    memcpy(m_state+112,&Aku,8);
    memcpy(m_state+120,&Ama,8);
    memcpy(m_state+128,&Ame,8);
    memcpy(m_state+136,&Ami,8);
    memcpy(m_state+144,&Amo,8);
    memcpy(m_state+152,&Amu,8);
    memcpy(m_state+160,&Asa,8);
    memcpy(m_state+168,&Ase,8);
    memcpy(m_state+176,&Asi,8);
    memcpy(m_state+184,&Aso,8);
    memcpy(m_state+192,&Asu,8);
    //copyToState(state, A)
    //Block::Put(NULL, state)(Aba)(Abe)(Abi)(Abo)(Abu)(Aga)(Age)(Agi)(Ago)(Agu)(Aka)(Ake)(Aki)(Ako)(Aku)(Ama)(Ame)(Ami)(Amo)(Amu)(Asa)(Ase)(Asi)(Aso)(Asu);
  }
}

void sha3_update(const unsigned char *input, unsigned int length)
{
  unsigned int spaceLeft;
  while (length >= (spaceLeft = 136 - m_counter))
  {
    xorbuf(m_state + m_counter, input, spaceLeft);
    KeccakF1600(m_state);
    input += spaceLeft;
    length -= spaceLeft;
    m_counter = 0;
  }

  xorbuf(m_state + m_counter, input, length);
  m_counter += (unsigned int)length;
}

void sha3_restart()
{
  memset(m_state, 0, 200);
  m_counter = 0;
}

void sha3_final(unsigned char *hash, unsigned int size)
{
  m_state[m_counter] ^= 1;
  m_state[135] ^= 0x80;
  KeccakF1600(m_state);
  memcpy(hash, m_state, size);
  sha3_restart();
}
