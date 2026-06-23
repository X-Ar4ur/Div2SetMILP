# Structural Evaluation by Generalized Integral Property?

Yosuke Todo 

NTT Secure Platform Laboratories, Tokyo, Japan todo.yosuke@lab.ntt.co.jp 

Abstract. In this paper, we show structural cryptanalyses against two popular networks, i.e., the Feistel Network and the Substitute-Permutation Network (SPN). Our cryptanalyses are distinguishing attacks by an improved integral distinguisher. The integral distinguisher is one of the most powerful attacks against block ciphers, and it is usually constructed by evaluating the propagation characteristic of integral properties, e.g., the ALL or BALANCE property. However, the integral property does not derive useful distinguishers against block ciphers with non-bijective functions and bit-oriented structures. Moreover, since the integral property does not clearly exploit the algebraic degree of block ciphers, it tends not to construct useful distinguishers against block ciphers with low-degree functions. In this paper, we propose a new property called the division property, which is the generalization of the integral property. It can effectively construct the integral distinguisher even if the block cipher has non-bijective functions, bit-oriented structures, and low-degree functions. From viewpoints of the attackable number of rounds or chosen plaintexts, the division property can construct better distinguishers than previous methods. Although our attack is a generic attack, it can improve several integral distinguishers against specific cryptographic primitives. For instance, it can reduce the required number of chosen plaintexts for the 10-round distinguisher on Keccak-f from 21025 to 2515. For the Feistel cipher, it theoretically proves that Simon 32, 48, 64, 96, and 128 have 9-, 11-, 11-, 13-, and 13-round integral distinguishers, respectively. 

Keywords: Block cipher, Integral distinguisher, Feistel Network, Substitute-Permutation Network, Keccak, Simon, AES-like cipher, Boolean function 

## 1 Introduction

The structural evaluation of cryptographic networks is an important topic of cryptology, and it helps a designer to design strong symmetric key primitives. There are several structural evaluations against the Feistel Network and the Substitute-Permutation Network (SPN) [BS01,IS13,Knu02,LR88,Pat04]. As one direction of the structural evaluation, there are the security evaluation by “the generic attack,” which exploits only the feature of the network and does not exploit the particular weaknesses of a specific cipher. It is applicable to large classes of block ciphers, but it is not often effective than the dedicated attack against the specific cipher. This paper focuses on generic attacks against both the Feistel Network and the SPN. The existing generic attack shows that the Feistel Network whose F -functions are chosen from random functions or permutations is vulnerable up to 5 rounds [Pat04,Knu02]. Moreover, Biryukov and Shamir showed that the SPN is vulnerable up to 2.5 rounds [BS01]. 

Our Contribution This paper shows generic attacks against two networks by improving an integral distinguisher. The integral attack was first proposed by Daemen et al. to evaluate the security of Square [DKR97], and then it was formalized by Knudsen and Wagner [KW02]. Nowadays, many integral distinguishers have been proposed against specific ciphers [KW02,LWZ11,WZ11,YPK02,ZRHD08], and they are often constructed by evaluating the propagation characteristic of integral properties, e.g., the ALL property or the BALANCE property. In this paper, we revisit the integral property, and then introduce the division property by generalizing the integral property. The division property can effectively construct integral distinguishers even if block ciphers have non-bijective functions, bit-oriented structures, and low-degree functions. 


Table 1. The number of required chosen plaintexts to construct r-round integral distinguishers on the Simon family, Serpent, and Keccak-f.


<table><tr><td rowspan="2">Target</td><td colspan="8"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td><td>r=12</td><td>r=13</td></tr><tr><td rowspan="2">SIMON 32</td><td>17</td><td>25</td><td>29</td><td>31</td><td>-</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="2">SIMON 48</td><td>17</td><td>29</td><td>39</td><td>44</td><td>46</td><td>47</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="2">SIMON 64</td><td>17</td><td>33</td><td>49</td><td>57</td><td>61</td><td>63</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="2">SIMON 96</td><td>17</td><td>33</td><td>57</td><td>77</td><td>87</td><td>92</td><td>94</td><td>95</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>33</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="2">SIMON 128</td><td>17</td><td>33</td><td>65</td><td>97</td><td>113</td><td>121</td><td>125</td><td>127</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>33</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td colspan="11"></td></tr><tr><td rowspan="2">Target</td><td colspan="8"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td></tr><tr><td rowspan="2">Serpent</td><td>12</td><td>28</td><td>84</td><td>113</td><td>124</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>82</td><td>113</td><td>123</td><td>127</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>BCC11</td></tr><tr><td colspan="11"></td></tr><tr><td rowspan="2">Target</td><td colspan="8"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td><td>r=12</td><td>r=13</td><td>r=14</td><td>r=15</td></tr><tr><td rowspan="2">KECCAK-f</td><td>130</td><td>258</td><td>515</td><td>1025</td><td>1410</td><td>1538</td><td>1580</td><td>1595</td><td>our</td><td>Sect. 5.3</td></tr><tr><td>257</td><td>513</td><td>1025</td><td>1409</td><td>1537</td><td>1579</td><td>1593</td><td>1598</td><td>degree</td><td>BCC11</td></tr></table>

The Feistel Network is a generic construction to create a (2`)-bit pseudo-random permutation from an `-bit pseudo-random function. We call the `-bit function the F -function, and assume that an attacker can not know the specification of the F -function. Our distinguishing attack can attack up to 3 rounds, and it can attack up to 5 rounds if the F -function is limited to a permutation. Unfortunately, they are not improved compared with the previous ones. However, assuming that the algebraic degree of the F -function is smaller than the bit length of the F -function, our attack can attack more rounds than the previous attacks exploiting the low-degree function. We summarize new integral distinguishers in Appendix B. Although the assumption of our attack is only the algebraic degree of the F -function, it can construct new integral distinguishers on the Simon family [BSS+13]. Since Simon has a non-bijective F -function and a bit-oriented structure, it is complicated task to construct the integral distinguisher. The division property theoretically introduces that Simon 32, 48, 64, 96, and 128 have at least 9-, 11-, 11-, 13-, and 13-round integral distinguishers, respectively. Table 1 shows the comparison between our distinguishers and previous ones. 

The SPN consists of an S-Layer and a P-Layer, where the S-Layer has m `-bit bijective S-boxes and the P-Layer has an (`m)-bit bijective linear function. The attacker can not know the specifications of the S-boxes and the linear function. Surprisingly, our generic attack becomes able to attack more rounds as the number of S-boxes is larger than the bit length of the S-box. This fact implies that the design of the P-Layer that can diffuse more outputs of S-boxes may not derive prospective security improvements. We summarize new integral distinguishers in Appendix C. Similar to the result against the Feistel Network, the division property is also useful to construct integral distinguishers against specific cryptographic primitives. For instance, we can reduce the required number of chosen plaintexts for the 7-round distinguisher on Serpent [ABK98] from $2 ^ { 1 2 7 }$ to $2 ^ { 1 2 4 }$ . Moreover, for the integral distinguisher on Keccak-f [DBPA11], we can reduce the required number of chosen plaintexts compared with previous ones constructed by Boura et al. [BCC11]. Table 1 shows the comparison between our distinguishers and previous ones. 

Organization This paper is organized as follows: In Sect. 2, we show notations, Boolean functions, and the framework of integral distinguishers. In Sect. 3, we propose the division property by generalizing the integral property, and show the propagation characteristic. In Sect. 4 and Sect. 5, we show new distinguishing attacks on the Feistel Network and the SPN, respectively. In Sect. 6, we show that the division property is also useful to construct the dedicated attack against specific ciphers. As an example, we show new distinguishing attacks on the AES-like cipher. Section 7 concludes this paper. 

## 2 Preliminaries

## 2.1 Notation

We make the distinction between addition of $\mathbb { F } _ { 2 } ^ { n }$ and addition of $\mathbb { Z } ,$ and we use  and + as addition of Fn2 and addition of $\mathbb { Z } ,$ respectively. For any $a \in \mathbb { F } _ { 2 } ^ { n }$ , the i-th element is expressed in $a [ i ]$ and the hamming weight $w _ { a }$ is calculated as $\textstyle w _ { a } = \sum _ { i = 1 } ^ { n } a [ i ]$ . Let $1 ^ { n } \in \mathbb { F } _ { 2 } ^ { n }$ be a value whose all elements are 1. Moreover, let $0 ^ { n } \in \mathbb { F } _ { 2 } ^ { n }$ be a value whose all elements are 0. 

$\mathbb { S } _ { k } ^ { n }$ $\mathbb { S } _ { k } ^ { n , m }$ $\mathbb { S } _ { k } ^ { n }$ $\mathbb { F } _ { 2 } ^ { n }$ $k \in \{ 0 , 1 , \ldots , n \}$ $\mathbb { S } _ { k } ^ { n }$ is a set of all $a \in \mathbb { F } _ { 2 } ^ { n }$ satisfying $k \leq w _ { a }$ , and it is defined as 

$$
\mathbb {S} _ {k} ^ {n} := \left\{a \in \mathbb {F} _ {2} ^ {n} \mid k \leq w _ {a} \right\}.
$$

$\mathbb { S } _ { k } ^ { n , m }$ $( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ $\pmb { k } \in ( \{ 0 , 1 , \dots , n \} ) ^ { m }$ $\mathbb { S } _ { k } ^ { n , m }$ is a set of all $\pmb { a } \in ( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ satisfying $k _ { i } \leq w _ { a _ { i } }$ , and it is defined as 

$$
\mathbb {S} _ {\boldsymbol {k}} ^ {n, m} := \left\{\left(a _ {1}, a _ {2}, \dots , a _ {m}\right) \in \left(\mathbb {F} _ {2} ^ {n}\right) ^ {m} \mid k _ {i} \leq w _ {a _ {i}} \text {for} 1 \leq i \leq m \right\}.
$$

Bit Product Functions $\pi _ { u }$ and $\pi _ { u }$ Let $\pi _ { u } : \mathbb { F } _ { 2 } ^ { n } \to \mathbb { F } _ { 2 }$ be a function for any $u \in \mathbb { F } _ { 2 } ^ { n }$ . Le t $x \in \mathbb { F } _ { 2 } ^ { n }$ be an input of $\pi _ { u }$ , and $\pi _ { u } ( x )$ is the AND of $x [ i ]$ satisfying $u [ i ] = 1$ , namely, it is defined as 

$$
\pi_ {u} (x) := \prod_ {i = 1} ^ {n} x [ i ] ^ {u [ i ]}.
$$

Let $\pi _ { \boldsymbol { u } } : ( \mathbb { F } _ { 2 } ^ { n } ) ^ { m } \to \mathbb { F } _ { 2 }$ be a function for any $\pmb { u } \in ( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ . Let $\pmb { x } \in ( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ be an input of $\pi _ { u }$ namely, $\pi _ { \boldsymbol { u } } ( \boldsymbol { x } )$ is calculated as 

$$
\pi_ {\boldsymbol {u}} (\boldsymbol {x}) := \prod_ {i = 1} ^ {m} \pi_ {u _ {i}} (x _ {i}).
$$

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/b915d7c9-7991-42a5-85b0-61d9f2301575/d8df74140eef6fc0a8718101fd520b0baf426169f2f2b3f7f22e4e0cc5c38d04.jpg)



Fig. 1. Integral distinguisher on 4-round AES


## 2.2 Boolean Function

A Boolean function is a function from $\mathbb { F } _ { 2 } ^ { n }$ to $\mathbb { F } _ { 2 } .$ . Let deg(f) be the algebraic degree of a Boolean function $f .$ As representations of the Boolean function, we use Algebraic Normal Form, which is defined as follows. 

Algebraic Normal Form Algebraic Normal Form (ANF) is a representation of a Boolean function. Any $f : \mathbb { F } _ { 2 } ^ { n } \to \mathbb { F } _ { 2 }$ can be represented as 

$$
f (x) = \bigoplus_ {u \in \mathbb {F} _ {2} ^ {n}} a _ {u} ^ {f} \left(\prod_ {i = 1} ^ {n} x [ i ] ^ {u [ i ]}\right) = \bigoplus_ {u \in \mathbb {F} _ {2} ^ {n}} a _ {u} ^ {f} \pi_ {u} (x),
$$

where $a _ { u } ^ { f } \in \mathbb { F } _ { 2 }$ is a constant value depending on f and u. If deg(f) is at most $d ,$ all $a _ { u } ^ { f }$ satisfying $d < w _ { u }$ are 0. An n-bit S-box can be regarded as the concatenation of n Boolean functions. If algebraic degrees of n Boolean functions are at most $d ,$ we say the algebraic degree of the S-box is at most $d .$ 

## 2.3 Integral Distinguisher

An integral distinguisher was first proposed by Daemen et al. to evaluate the security of Square [DKR97], and then it was formalized by Knudsen and Wagner [KW02]. It uses a set of chosen plaintexts that contains all possible values for some bits and has a constant value for the other bits. Corresponding ciphertexts are calculated from plaintexts in the set by using an encryption oracle. If the XOR of the corresponding ciphertexts always becomes 0, we say that this cipher has the integral distinguisher. 

Integral Property Nowadays, many integral distinguishers have been proposed against specific ciphers [KW02,LWZ11,WZ11,YPK02,ZRHD08], and they are often constructed by evaluating the propagation characteristic of the integral property. We define four integral properties as follows: 

– ALL (A) : Every value appears the same number in the multiset. 

– BALANCE ( ) : The XOR of all texts in the multiset is 0. 

– CONSTANT ( ) : The value is fixed to a constant for all texts in the multiset. 

– UNKNOWN (U ) : The multiset is indistinguishable from one of n-bit random values. 

Knudsen and Wagner showed that AES has the 4-round integral distinguisher with $2 ^ { 3 2 }$ chosen plaintexts [KW02]. Figure 1 shows the integral distinguisher. 

Unfortunately, the integral property does not derive effective distinguishers if block ciphers consist of non-bijective functions, e.g., DES [U.S77] and Simon [BSS+13] consist of non-bijection functions. Moreover, since the propagation characteristic does not clearly exploit the algebraic degree of block ciphers, it tends not to construct effective distinguishers against block ciphers with low-degree round functions. 

Degree Estimation As another method to construct the integral distinguisher, there is a higher-order differential attack [Lai94,Knu94], which exploits the algebraic degree of block ciphers. When the algebraic degree of a block cipher is at most D, the cipher has the integral distinguisher with $2 ^ { { \bar { D } } + 1 }$ chosen plaintexts. Canteaut and Videau showed the bound of the degree of iterated round functions [CV02]. Then, Boura et al. improved the bound [BCC11], and showed integral distinguishers on Keccak [DBPA11] and $L u f f a$ [CSW08]. We show the bound in Appendix A. 

## 3 Division Property

## 3.1 Introduction of Division Property

We propose a new property called the division property, which is the generalization of the integral property. We consider one bijective S-box with degree d. If an input multiset has A, the output multiset also has A. If an input multiset has B, the output multiset has U . If we have the input multiset with $2 ^ { d + 1 }$ chosen texts, the output multiset has B because the degree of the S-box is d. The integral property does not exploit this property. We now want to exploit useful properties that are hidden between A and B. Therefore, we redefine A and B by the same notation, and then introduce the division property by generalizing the redefinition. 

Redefinition of Integral Property Let X be a multiset whose elements take an n-bit value. We first consider features of the multiset X satisfying A. If we choose one bit from n bits and calculate the XOR of the chosen bit in the multiset, the calculated value is always 0. Moreover, if we choose at most $( n - 1 )$ bits from n bits and calculate the XOR of the AND of chosen bits in the multiset, the calculated value is also always 0. However, if we choose all bits from n bits and calculate the XOR of the AND of n bits in the multiset, the calculated value becomes unknown1. Above features are expressed by using the bit product function $\pi _ { u }$ , which is defined in Sect. 2.1, as follows. We evaluate the parity of $\pi _ { u } ( x )$ for all $x \in \mathbb { X }$ , namely, evaluate $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ . The parity is always even for any u satisfying $w _ { u } < n$ . On the other hand, the parity becomes unknown for $u = 1 ^ { n }$ . 

We next consider features of the multiset X satisfying . If we choose one bit from n bits and calculate the XOR of the chosen bit in the multiset, the calculated value is always 0. However, if we choose at least two bits from n bits and calculate the XOR of the AND of chosen bits in the multiset, the calculated value becomes unknown. Above features are expressed by using the bit product function $\pi _ { u }$ as follows. We evaluate the parity of $\pi _ { u } ( x )$ for all $x \in \mathbb { X }$ . The parity is always even for any u satisfying $w _ { u } < 2 . \mathrm { O n }$ the other hand, the parity becomes unknown for any u satisfying $w _ { u } \ge 2$ . 

## 3.2 Definition of Division Property

Section 3.1 redefines both the ALL and BALANCE properties by the same notation. Since the redefinition can be parameterized by the number of product bits $w _ { u }$ of the bit product function $\pi _ { u } ,$ we generalize the integral property as follows. 

Definition 1 (Division Property). Let X be a multiset whose elements take a value of $\mathbb { F } _ { 2 } ^ { n }$ , and k takes a value between 0 and n. When the multiset $\mathbb { X }$ has the division property $\mathcal { D } _ { k } ^ { n }$ , it fulfils the following conditions: The parity $o f \pi _ { u } ( x )$ for all $x \in \mathbb { X }$ is always even $i f w _ { u }$ is less than k. Moreover, the parity becomes unknown if $w _ { u }$ is greater than or equal to k. 

When the multiset X has $\mathcal { D } _ { k } ^ { n }$ , it satisfies 

$$
\bigoplus_ {x \in \mathbb {X}} \pi_ {u} (x) = 0, \text {   for   all   } u \in (\mathbb {F} _ {2} ^ {n} \setminus \mathbb {S} _ {k} ^ {n}),
$$

where $\mathbb { S } _ { k } ^ { n }$ is a subset defined in Sect. 2.1. The parity of $\pi _ { u } ( x )$ for all $x \in \mathbb { X }$ becomes unknown for any $u \in \mathbb { S } _ { k } ^ { n }$ . Namely, in the division property, the set of u is divided into the subset that $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ becomes unknown and the subset that $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ becomes 0. 

Example 1. Let X be a multiset whose elements take a value of $\mathbb { F } _ { 2 } ^ { 4 } . \mathrm { ~ A s ~ }$ an example, we prepare the input multiset $\mathbb { X }$ as 

$$
\mathbb {X} := \left\{0 x 0, 0 x 3, 0 x 3, 0 x 3, 0 x 5, 0 x 6, 0 x 8, 0 x B, 0 x D, 0 x E \right\}.
$$

A following table calculates the summation of $\pi _ { u } ( x )$ . 

<table><tr><td rowspan="2"></td><td>0x0</td><td>0x3</td><td>0x3</td><td>0x3</td><td>0x5</td><td>0x6</td><td>0x8</td><td>0xB</td><td>0xD</td><td>0xE</td><td rowspan="2"><eq>\sum {\pi }_{u}\left( x\right)</eq><eq>\left( {\bigoplus {\pi }_{u}\left( x\right) }\right)</eq></td></tr><tr><td>0000</td><td>0011</td><td>0011</td><td>0011</td><td>0101</td><td>0110</td><td>1000</td><td>1011</td><td>1101</td><td>1110</td></tr><tr><td><eq>u = {0000}</eq></td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>10 (0)</td></tr><tr><td><eq>u = {0001}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>6 (0)</td></tr><tr><td><eq>u = {0010}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>1</td><td>0</td><td>1</td><td>0</td><td>1</td><td>6 (0)</td></tr><tr><td><eq>u = {0011}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>4 (0)</td></tr><tr><td><eq>u = {0100}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>0</td><td>1</td><td>1</td><td>4 (0)</td></tr><tr><td><eq>u = {0101}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>2 (0)</td></tr><tr><td><eq>u = {0110}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>2 (0)</td></tr><tr><td><eq>u = {0111}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0 (0)</td></tr><tr><td><eq>u = {1000}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>1</td><td>1</td><td>4 (0)</td></tr><tr><td><eq>u = {1001}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>2 (0)</td></tr><tr><td><eq>u = {1010}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1</td><td>2 (0)</td></tr><tr><td><eq>u = {1011}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>1 (1)</td></tr><tr><td><eq>u = {1100}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>2 (0)</td></tr><tr><td><eq>u = {1101}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1 (1)</td></tr><tr><td><eq>u = {1110}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1 (1)</td></tr><tr><td><eq>u = {1111}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0 (0)</td></tr></table>

For all u satisfying $w _ { u } < 3 , \oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ becomes 0. Therefore, the multiset has the division property $\mathcal { D } _ { 3 } ^ { 4 }$ . 

Each definition of  and  is essentially the same as that of $\mathcal { D } _ { 2 } ^ { n }$ and $\mathcal { D } _ { 1 } ^ { n }$ , respectively. However, the definition of A is different from that of $\mathcal { D } _ { n } ^ { n }$ . The multiset satisfying A always has the division property $\mathcal { D } _ { n } ^ { n }$ but not vice versa. For instance, the multiset satisfying the 

EVEN property, which is defined that the number of occurrences is even for all values [SK12], does not always have ${ \mathcal { A } } ,$ but it always has $\mathcal { D } _ { n } ^ { n }$ . In this paper, we use only $\mathcal { D } _ { n } ^ { n }$ instead of A because it is sufficient to use $\mathcal { D } _ { n } ^ { n }$ from the viewpoint of the construction of integral distinguishers. 

Propagation Characteristic of Division Property Let s be an S-box whose degree is d. Let X be an input multiset whose elements take a value of Fn2 . Let $\mathbb { Y }$ be an output $s ( x )$ $x \in \mathbb { X }$ $\mathcal { D } _ { k } ^ { n }$ and want to evaluate the division property of Y. In the division property, the set of u is divided into the subset that $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ becomes unknown and the subset that $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ becomes 0. Therefore, we divide the set of v into the subset that $\begin{array} { r } { \bigoplus _ { s ( x ) \in \mathbb { Y } } \pi _ { v } \bigl ( s ( x ) \bigr ) } \end{array}$ becomes unknown and the subset that $\textstyle \bigoplus _ { s ( x ) \in \mathbb { Y } } \pi _ { v } { \bigl ( } s ( x ) { \bigr ) }$ becomes 0. Since the parity of $\pi _ { v } ( s ( x ) )$ for all $s ( x ) \in \mathbb { Y }$ is equal to that of $( \pi _ { v } \circ s ) ( x )$ for all $x \in \mathbb { X }$ , we evaluate $\oplus _ { x \in \mathbb { X } } ( \pi _ { v } \circ s ) ( x )$ . 

Proposition 1 (Propagation Characteristic of Division Property). Let s be an function (S-box) from n bits to n bits, and the degree is d. Assuming that an input multiset X has the division property $\mathcal { D } _ { k } ^ { n }$ , the output multiset Y has $\mathcal { D } _ { \lceil \frac { k } { d } \rceil } ^ { n }$ In addition, assuming that the S-box is a permutation, the output multiset Y has $\mathcal { D } _ { n } ^ { n }$ when the input multiset has $\mathcal { D } _ { n } ^ { n }$ . 

Proof. We represent $\oplus _ { x \in \mathbb { X } } ( \pi _ { v } \circ s ) ( x )$ by using ANF as 

$$
\begin{array}{l} \bigoplus_ {x \in \mathbb {X}} (\pi_ {v} \circ s) (x) = \bigoplus_ {x \in \mathbb {X}} \left(\bigoplus_ {u \in \mathbb {F} _ {2} ^ {n}} a _ {u} ^ {\pi_ {v} \circ s} \pi_ {u} (x)\right) \\ = \bigoplus_ {u \in \mathbb {S} _ {k} ^ {n}} a _ {u} ^ {\pi_ {v} \circ s} \left(\bigoplus_ {x \in \mathbb {X}} \pi_ {u} (x)\right) \oplus \bigoplus_ {u \in (\mathbb {F} _ {2} ^ {n} \setminus \mathbb {S} _ {k} ^ {n})} a _ {u} ^ {\pi_ {v} \circ s} \left(\bigoplus_ {x \in \mathbb {X}} \pi_ {u} (x)\right). \\ \end{array}
$$

Since the multiset X has $\mathcal { D } _ { k } ^ { n } , \bigoplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ is always 0 for any $u \in ( \mathbb { F } _ { 2 } ^ { n } \setminus \mathbb { S } _ { k } ^ { n } )$ . Therefore, it satisfies 

$$
\bigoplus_ {x \in \mathbb {X}} (\pi_ {v} \circ s) (x) = \bigoplus_ {u \in \mathbb {S} _ {k} ^ {n}} a _ {u} ^ {\pi_ {v} \circ s} \left(\bigoplus_ {x \in \mathbb {X}} \pi_ {u} (x)\right).
$$

If $a _ { u } ^ { \pi _ { v } \circ s }$ is 0 for all $u \in \mathbb { S } _ { k } ^ { n } , \bigoplus _ { x \in \mathbb { A } } ( \pi _ { v } \circ s ) ( x )$ always becomes 0. In other words, if there exists $u \in \mathbb { S } _ { k } ^ { n }$ such that $a _ { u } ^ { \pi _ { v } \circ s }$ is $1 , \oplus _ { x \in \mathbb { A } } ( \pi _ { v } \circ s ) ( x )$ becomes unknown. Since the function $\pi _ { v }$ is the AND of $w _ { v }$ bits and the degree of S-box is $d ,$ the degree of the Boolean function $( \pi _ { v } \circ s )$ has the following properties: 

– The degree of $( \pi _ { v } \circ s )$ is at most min $\{ n , w _ { v } \times d \}$ . 

– If the S-box is a permutation, the degree of $( \pi _ { v } \circ s )$ is at most $n - 1$ for $w _ { v } < n$ 

We first assume that the multiset X has $\mathcal { D } _ { k } ^ { n }$ . In this case, we consider only u satisfying $w _ { u } \ge k$ . When $w _ { v } \times d < k$ holds, $a _ { u } ^ { \pi _ { v } \circ s }$ is always 0. Thus, the necessary condition that $a _ { u } ^ { \pi _ { v } \circ s }$ $w _ { v } \times d \geq k$ $w _ { v } \geq \lceil \frac { k } { d } \rceil$ $\oplus _ { x \in \mathbb { X } } ( \pi _ { v } \circ s ) ( x )$ becomes unknown is $w _ { v } \geq \lceil \frac { k } { d } \rceil$ , and Y has $\mathcal { D } _ { \lceil \frac { k } { d } \rceil } ^ { n }$ . We next assume that the multiset X has $\mathcal { D } _ { n } ^ { n }$ and the S-box is a permutation. In this case, we consider only $u = 1 ^ { n }$ . $w _ { v } < n$ $a _ { 1 ^ { n } } ^ { \pi _ { v } \circ s }$ $( \pi _ { v } \circ s )$ at most $n - 1$ . Thus, the necessary condition that $a _ { 1 ^ { n } } ^ { \pi _ { v } \circ s }$ becomes 1 is $v = 1 ^ { n }$ . Namely, the necessary condition that $\oplus _ { x \in \mathbb { X } } ( \pi _ { v } \circ s ) ( x )$ becomes unknown is $v = 1 ^ { n }$ , and Y has $\mathcal { D } _ { n } ^ { n }$ . □ 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/b915d7c9-7991-42a5-85b0-61d9f2301575/40827aa1dddebbb8b2dc9cf4815bafa6421b87da08d096e46821a1e47f53c31a.jpg)



Fig. 2. Propagation characteristic of division property


Example 2. Let us consider a following 4-bit S-box. 

<table><tr><td>x</td><td>0x0</td><td>0x1</td><td>0x2</td><td>0x3</td><td>0x4</td><td>0x5</td><td>0x6</td><td>0x7</td><td>0x8</td><td>0x9</td><td>0xA</td><td>0xB</td><td>0xC</td><td>0xD</td><td>0xE</td><td>0xF</td></tr><tr><td>s(x)</td><td>0x8</td><td>0xC</td><td>0x0</td><td>0xB</td><td>0x9</td><td>0xD</td><td>0xE</td><td>0x5</td><td>0xA</td><td>0x1</td><td>0x2</td><td>0x6</td><td>0x4</td><td>0xF</td><td>0x3</td><td>0x7</td></tr></table>

The S-box is bijective and the algebraic degree is 2. We now prepare the input multiset X as 

$$
\mathbb {X} := \{0 x 0, 0 x 3, 0 x 3, 0 x 3, 0 x 5, 0 x 6, 0 x 8, 0 x B, 0 x D, 0 x E \},
$$

which is the same as Example 1 and the division property is $\mathcal { D } _ { 3 } ^ { 4 }$ . The output multiset is calculated as 

$$
\mathbb {Y} := \left\{0 x 8, 0 x B, 0 x B, 0 x B, 0 x D, 0 x E, 0 x A, 0 x 6, 0 x F, 0 x 3 \right\},
$$

and a following table calculates the summation of $\pi _ { v } ( y )$ . 

<table><tr><td rowspan="2"></td><td>0x8</td><td>0xB</td><td>0xB</td><td>0xB</td><td>0xD</td><td>0xE</td><td>0xA</td><td>0x6</td><td>0xF</td><td>0x3</td><td rowspan="2"><eq>\sum {\pi }_{v}\left( y\right)</eq><eq>\left( {\bigoplus {\pi }_{v}\left( y\right) }\right)</eq></td></tr><tr><td>1000</td><td>1011</td><td>1011</td><td>1011</td><td>1101</td><td>1110</td><td>1010</td><td>0110</td><td>1111</td><td>0011</td></tr><tr><td><eq>v = {0000}</eq></td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>10 (0)</td></tr><tr><td><eq>v = {0001}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>6 (0)</td></tr><tr><td><eq>v = {0010}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>8 (0)</td></tr><tr><td><eq>v = {0011}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>5 (1)</td></tr><tr><td><eq>v = {0100}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>1</td><td>1</td><td>0</td><td>4 (0)</td></tr><tr><td><eq>v = {0101}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>2 (0)</td></tr><tr><td><eq>v = {0110}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1</td><td>1</td><td>0</td><td>3 (1)</td></tr><tr><td><eq>v = {0111}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1 (1)</td></tr><tr><td><eq>v = {1000}</eq></td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>0</td><td>1</td><td>0</td><td>8 (0)</td></tr><tr><td><eq>v = {1001}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>5 (1)</td></tr><tr><td><eq>v = {1010}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>1</td><td>1</td><td>0</td><td>1</td><td>0</td><td>6 (0)</td></tr><tr><td><eq>v = {1011}</eq></td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>4 (0)</td></tr><tr><td><eq>v = {1100}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>0</td><td>1</td><td>0</td><td>3 (1)</td></tr><tr><td><eq>v = {1101}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>2 (0)</td></tr><tr><td><eq>v = {1110}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>1</td><td>0</td><td>2 (0)</td></tr><tr><td><eq>v = {1111}</eq></td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1 (1)</td></tr></table>

For all v satisfying $w _ { v } \ < 2 , \oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ becomes 0. Therefore, the multiset $\mathbb { Y }$ has the division property $\mathcal { D } _ { 2 } ^ { 4 }$ . 

Figure 2 shows the outline of the propagation characteristic of the division property. Let X and Y be input and output multisets, respectively. First, the size of the set of u that $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( x )$ becomes unknown is small. However, the size of the set of u that $\oplus _ { x \in \mathbb { X } } \pi _ { u } ( s ( x ) )$ becomes unknown expands. If the size expands to the universal set except for $0 ^ { n }$ , we regard that the output multiset is indistinguishable from the multiset of random texts. 

## 3.3 Vectorial Division Property

Section 3.2 only shows the division property for one S-box. However, since practical ciphers use several S-boxes in every round, we can not construct integral distinguishers by only using Proposition 1. Therefore, we vectorize the division property. 

Let an S-Layer be any function that consists of m n-bit S-boxes with degree d in parallel. We now consider the propagation characteristic of the division property against the S-Layer. Let X be the input multiset of the S-Layer, and $x \in \mathbb { X }$ takes a value of $( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ . The vectorization is the natural extension of the division property. Namely, the set of u is divided into the subset that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ becomes unknown and the subset that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ becomes 0, where u is an m-dimensional vector whose elements take a value of $\mathbb { F } _ { 2 } ^ { n }$ . Figure 3 shows the difference between the division property and the vectorial one. 

Definition 2 (Vectorial Division Property). Let X be the multiset whose elements take a value of $( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ , and k is an m-dimensional vector whose elements take a value between 0 $\mathcal { D } _ { k } ^ { n , m }$ $\pi _ { \boldsymbol { u } } ( \boldsymbol { x } )$ $\pmb { x } \in \mathbb { X }$ $i f$ $\mathbb { S } _ { k } ^ { n , m }$ $t o \mathbb { S } _ { \pmb { k } } ^ { n , m }$ 

Propagation Characteristic of Vectorial Division Property Assume that the input multiset of the S-Layer has the division property $\mathcal { D } _ { k } ^ { n , m }$ . The output of the S-Layer is calculated as $S ( \pmb { x } ) = ( s _ { 1 } ( x _ { 1 } ) , s _ { 2 } ( x _ { 2 } ) , \dots , s _ { m } ( x _ { m } ) )$ for $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } ) \in \mathbb { X }$ . We now consider the set of v that $\textstyle \bigoplus _ { x \in \mathbb { X } } \pi _ { v } ( S ( { \pmb x } ) )$ becomes unknown and the set of v that $\textstyle \bigoplus _ { x \in \mathbb { X } } \pi _ { v } ( S ( { \pmb x } ) )$ becomes 0. Since the output of each S-box is calculated independently, the propagation characteristic of the division property can also be evaluated independently. Namely, the output multiset has Dn,mk0 , $\mathcal { D } _ { k ^ { \prime } } ^ { n , m }$ where $k _ { i } ^ { \prime } = \lceil k _ { i } / d \rceil$ holds. Moreover, if the S-box is bijective and $k _ { i } = n$ holds, $k _ { i } ^ { \prime } = n$ holds. 

## 3.4 Collective Division Property

By vectorizing of the division property, we can evaluate the multiset whose elements take a value of $( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ . However, it is still insufficient to use only vectorial division property. For simplicity, we consider a multiset X whose elements take a value of $( \mathbb { F } _ { 2 } ^ { 8 } ) ^ { 2 }$ . Assume that the number of elements in X is 256, and two elements of x take all values from 0 to 255 independently. We consider the set of u that the parity of $\pi _ { \boldsymbol { u } } ( \boldsymbol { x } )$ for all $\textbf { \em x } \in \mathrm { ~ \mathbb ~ X ~ }$ becomes unknown and the set of u that the parity becomes 0. 

– The parity becomes unknown if u belongs to $\mathbb { S } _ { [ 8 , 0 ] } ^ { 8 , 2 }$ . 

– The parity becomes unknown if u belongs to S8,2 $\mathbb { S } _ { [ 0 , 8 ] } ^ { 8 , 2 }$ . 

– The parity becomes unknown if u belongs to $\mathbb { S } _ { [ 1 , 1 ] } ^ { 8 , 2 } .$ 

– Otherwise, i.e., u does NOT belong t o S8,2 $\mathbb { S } _ { [ 8 , 0 ] } ^ { 8 , 2 } \cup \dot { \mathbb { S } } _ { [ 0 , 8 ] } ^ { 8 , 2 } \cup \mathbb { S } _ { [ 1 , 1 ] } ^ { 8 , 2 }$ the parity is always even. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/b915d7c9-7991-42a5-85b0-61d9f2301575/cfab113ffffc30d66960d90fa5b7b69014c3616ce0228b7164a43258022d12f8.jpg)



Fig. 3. Division property, vectorial division property, and collective division property


We can not express this property by using the vectorial division property. Therefore, we collect several vectorial division properties. Figure 3 shows the difference between the vectorial division property and the collective division property. 

Definition 3 (Collective Division Property). Let X be the multiset whose elements take a value $o f \ ( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ , and $\pmb { k } ^ { ( j ) } \ ( j = 1 , 2 , \ldots , q )$ are m-dimensional vectors whose elements $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { n , m }$ the multiset fulfils the following conditions: The parity of $\pi _ { \boldsymbol { u } } ( \boldsymbol { x } )$ for all $\pmb { x } \in \mathbb { X }$ is always even $\mathbb { S } _ { \pmb { k } ^ { ( 1 ) } } ^ { n , m } \cup \mathbb { S } _ { \pmb { k } ^ { ( 2 ) } } ^ { n , m } \cup \cdots \cup \mathbb { S } _ { \pmb { k } ^ { ( q ) } } ^ { n , m }$ . Moreover, the parity becomes unknown if u belongs to the union $\mathbb { S } _ { \pmb { k } ^ { ( 1 ) } } ^ { n , m } \cup \mathbb { S } _ { \pmb { k } ^ { ( 2 ) } } ^ { n , m } \cup \cdots \cup \mathbb { S } _ { \pmb { k } ^ { ( q ) } } ^ { n , m }$ . 

It is obvious that the collective division property with $q = 1$ is the same as the vectorial division property. 

Propagation Characteristic of Collective Division Property Assume that the input multiset of the S-Layer has the division property $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { n , m }$ Dn,mk(1),k(2),...,k(q) . We now consider the set of v that $\textstyle \bigoplus _ { x \in \mathbb { X } } \pi _ { v } ( S ( { \pmb x } ) )$ becomes unknown, and the set is derived from only the set of u that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ of Dn,m(j) $\bar { \mathcal { D } } _ { k ^ { ( j ) } } ^ { n , m }$ $\mathcal { D } _ { k ^ { \prime ( 1 ) } , k ^ { \prime ( 2 ) } , \ldots , k ^ { \prime ( q ) } } ^ { n , m }$ D k0(1),k0(2),...,k0(q) , where $k _ { i } ^ { \prime ( j ) } = \lceil k _ { i } ^ { ( j ) } / d \rceil$ $k _ { i } ^ { ( j ) } = n$ $k _ { i } ^ { \prime ( j ) } = n$ holds. 

## 4 Improved Integral Distinguishers on Feistel Network

## 4.1 Feistel Network

(`, d)-Feistel The Feistel Network is one of the most popular network to design block ciphers. When n-bit block ciphers are constructed by the Feistel Network, the input of the round function is expressed in two $( n / 2 )$ -bit values. Moreover, an $( n / 2 )$ -bit non-linear function F is used in the round function, and we call this function the F -function. Let $( w _ { 1 } , w _ { 2 } )$ be the input of the round function, and the output is calculated as $( z _ { 1 } , z _ { 2 } ) =$ $( F ( w _ { 1 } ) \oplus w _ { 2 } , w _ { 1 } )$ . We now define an $( \ell , d )$ -Feistel, whose F -function is an `-bit non-linear function with degree d (this function is not limited to a permutation). Figure 4 shows the round function of the Feistel Network. There are many block ciphers adopting $( \ell , d ) { \mathrm { - F e i s t e l } } .$ , e.g. DES [U.S77], Camellia [AIK+00], and Simon 2n [BSS+13] adopt (32, 5)-, (64, 7)-, and (n, 2)-Feistel, respectively. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/b915d7c9-7991-42a5-85b0-61d9f2301575/73e989e93aacc101936419ebacabaaaa09ed8b56f0a76aee27ab93de3f20ecab.jpg)



Fig. 4. (`, d)-Feistel


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/b915d7c9-7991-42a5-85b0-61d9f2301575/657ee7f04ac388cfde5a0a6ced3a71abe49d86e831d528f52314eceed2070f0e.jpg)



Fig. 5. Propagation characteristic for Feistel Network


## 4.2 Propagation Characteristic for Feistel Network

This section shows that the division property is useful to construct integral distinguishers on $( \ell , d )$ -Feistel. Since the Feistel Network has “copy,” “substitution,” and “compression,” we need to propagate the division property against them. The “copy” creates the input of the F -function, and the “substitution” processes the input by the F-function, and finally the “compression” creates the left half of the output by XOR. Figure 5 shows the outline of the propagation characteristic. 

-1- Copy Let W be an input set, and $( w _ { 1 } , w _ { 2 } ) \in \mathbb { W }$ denotes the input value. The round function first creates $( x _ { 1 } , x _ { 2 } , x _ { 3 } )$ , where $x _ { 1 } = w _ { 1 } , x _ { 2 } = w _ { 1 }$ , and $x _ { 3 } = w _ { 2 }$ hold. ${ \mathrm { H e r e } } , x _ { 1 }$ is the input of the F -function, $x _ { 2 }$ is the right half of the output of the round function, and $x _ { 3 }$ is the right half of the input of the round function. Let X be the output set whose elements take $( x _ { 1 } , x _ { 2 } , x _ { 3 } )$ for all $( w _ { 1 } , w _ { 2 } ) \in \mathbb { W }$ . Assume that the input set W has the division property D`,2k(1), $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { \ell , 2 }$ . If we use $\pi _ { \pmb { u } }$ satisfying $k _ { 1 } ^ { ( j ) } \leq u _ { 1 }$ and $k _ { 2 } ^ { ( j ) } \leq u _ { 2 }$ , the parity of $\pi _ { u } ( w )$ for all $\pmb { w } \in \mathbb { W }$ becomes unknown. Since $x _ { 1 }$ is equal to $x _ { 2 }$ , the parity of $\pi _ { \pmb { v } } ( \pmb { x } )$ for all $\pmb { x } \in \mathbb { X }$ becomes unknown if we use $\pi _ { v }$ satisfying $k _ { 1 } ^ { ( j ) } - k ^ { \prime } \leq v _ { 1 } , k ^ { \prime } \leq v _ { 2 }$ , $k _ { 2 } ^ { ( j ) } \le v _ { 3 }$ $k ^ { \prime } ~ ( 0 \leq k ^ { \prime } \leq k _ { 1 } ^ { ( j ) } )$ `,3 $\mathcal { D } ^ { \ell , 3 }$ $\begin{array} { r l r } {  { \mathcal { D } _ { [ 0 , k _ { 1 } ^ { ( 1 ) } , k _ { 2 } ^ { ( 1 ) } ] , [ 1 , k _ { 1 } ^ { ( 1 ) } - 1 , k _ { 2 } ^ { ( 1 ) } ] , \ldots , [ k _ { 1 } ^ { ( 1 ) } , 0 , k _ { 2 } ^ { ( 1 ) } ] , \ldots , [ k _ { 1 } ^ { ( q ) } , 0 , k _ { 2 } ^ { ( q ) } ] } ^ { - \gamma ^ { ( - 1 ) } } . } } \end{array}$ 

-2- Substitution The F -function is an `-bit function with degree d. Assume that the $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { \ell , 3 }$ D`,3k(1),k(2),...,k(q) . From the propagation characteristic of the division property, the output set ha s `,3 $\mathcal { D } _ { \pmb { k } ^ { \prime ( 1 ) } , \pmb { k } ^ { \prime ( 2 ) } , \dots , \pmb { k } ^ { \prime ( q ) } } ^ { \ell , 3 } ,$ Dk0(1),k0(2),...,k0(q) , where ( $( k _ { 1 } ^ { \prime ( j ) } , k _ { 2 } ^ { \prime ( j ) } , k _ { 3 } ^ { \prime ( j ) } ) =$ $( \lceil k _ { 1 } ^ { ( j ) } / d \rceil , k _ { 2 } ^ { ( j ) } , k _ { 3 } ^ { ( j ) } )$ $k _ { 1 } ^ { \prime ( j ) }$ when $k _ { 1 } ^ { ( j ) } = \ell$ holds. 

-3- Compression Let Y be the input set, and $( y _ { 1 } , y _ { 2 } , y _ { 3 } ) \in \mathbb { Y }$ denotes the input value, where $y _ { 1 }$ denotes the output of the F -function. Let $y _ { 1 }$ be XORed with $y _ { 3 } ,$ , and then the internal state is expressed in $( z _ { 1 } , z _ { 2 } ) = ( y _ { 1 } \oplus y _ { 3 } , y _ { 2 } )$ . Let $\mathbb { Z }$ be the set whose elements take $( z _ { 1 } , z _ { 2 } )$ for all $( y _ { 1 } , y _ { 2 } , y _ { 3 } ) \in \mathbb { Y }$ . To evaluate the division property of the set $\mathbb { Z } ,$ we calculate the parity of $\pi _ { v } ( z _ { 1 } , z _ { 2 } )$ for all $( z _ { 1 } , z _ { 2 } ) \in \mathbb { Z }$ as 

$$
\begin{array}{l} \bigoplus_ {(z _ {1}, z _ {2}) \in \mathbb {Z}} \pi_ {\boldsymbol {v}} (z _ {1}, z _ {2}) = \bigoplus_ {(z _ {1}, z _ {2}) \in \mathbb {Z}} (\pi_ {v _ {1}} (z _ {1}) \times \pi_ {v _ {2}} (z _ {2})) \\ = \bigoplus_ {(y _ {1}, y _ {2}, y _ {3}) \in \mathbb {Y}} (\pi_ {v _ {1}} (y _ {1} \oplus y _ {3}) \times \pi_ {v _ {2}} (y _ {2})) \\ = \bigoplus_ {(y _ {1}, y _ {2}, y _ {3}) \in \mathbb {Y}} \left(\bigoplus_ {c \preceq v _ {1}} (\pi_ {c} (y _ {1}) \times \pi_ {v _ {1} \oplus c} (y _ {3})) \times \pi_ {v _ {2}} (y _ {2})\right) \\ = \bigoplus_ {c \preceq v _ {1}} \left(\bigoplus_ {(y _ {1}, y _ {2}, y _ {3}) \in \mathbb {Y}} \pi_ {c} (y _ {1}) \times \pi_ {v _ {2}} (y _ {2}) \times \pi_ {v _ {1} \oplus c} (y _ {3})\right), \\ \end{array}
$$

where the set of c chosen from $c \preceq v _ { 1 }$ denotes the set of c satisfying $c \wedge v _ { 1 } = c .$ Assuming that the input set Y has the division property $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { \ell , 3 }$ D`,3k(1),k(2),...,k(q) , the output set Z has the division property $\mathcal { D } _ { { \pmb k } ^ { \prime ( 1 ) } , { \pmb k } ^ { \prime ( 2 ) } , \ldots , { \pmb k } ^ { \prime ( q ) } } ^ { \ell , 2 }$ D`,2k0(1),k0(2),...,k0(q) , where (k0(j)1 , $( k _ { 1 } ^ { \prime ( j ) } , k _ { 2 } ^ { \prime ( j ) } ) = ( k _ { 1 } ^ { ( j ) } + k _ { 3 } ^ { ( j ) } , k _ { 2 } ^ { ( j ) } )$ that the parity of $\pi _ { v } ( z _ { 1 } , z _ { 2 } )$ for all $( z _ { 1 } , z _ { 2 } ) \in \mathbb { Z }$ becomes 0 if $k _ { 1 } ^ { ( j ) } + k _ { 3 } ^ { ( j ) }$ ( is more than `. 

## 4.3 Path Search Algorithm for (`, d)-Feistel

This section shows the path search algorithm for integral distinguishers against $( \ell , d ) { \mathrm { - F e i s t e l } } .$ . The algorithm is based on the propagation characteristic shown in Sect. 4.2. Assume that $k _ { 1 }$ bits of the left half of the input are active and the rest $( \ell - k _ { 1 } )$ bits are constant. Moreover, assume that $k _ { 2 }$ bits of the right half of the input are active and the rest $( \ell -$ $k _ { 2 } )$ bits are constant. Namely, we prepare $2 ^ { k _ { 1 } + k _ { 2 } }$ chosen plaintexts. The input set has $\mathcal { D } _ { [ k _ { 1 } , k _ { 2 } ] } ^ { \ell , 2 }$ D`,2[k1,k2]. Algorithm 1 shows the path search algorithm to create the integral distinguisher on $( \ell , \dot { d } )$ -Feistel. Algorithm 1 does not limit the F -function to be a permutation. If the F -function is limited to be a permutation, L becomes $k _ { 2 } + \ell$ when $X = \ell$ holds (see the 4-th line in Algorithm 1). Algorithm 1 calls SizeReduce, which eliminates $\boldsymbol { k } ^ { ( i , j ) }$ $( i ^ { \prime } , j ^ { \prime } )$ $\mathbb { S } _ { k ^ { ( i , j ) } } ^ { \overline { { \ell , 2 } } } \subseteq \mathbb { S } _ { k ^ { ( i ^ { \prime } , j ^ { \prime } ) } } ^ { \ell , 2 } .$ ) ⊆ S`,2(i0,j 

Results Table 2 shows the number of required chosen plaintexts to construct r-round integral distinguishers on (32, 5)- and (64, 7)-Feistel, where DES [U.S77] is classified into (32, 5)-Feistel with non-bijective function and Camellia [AIK+00] is classified into (64, 7)- Feistel with bijective function. When we construct the integral distinguisher on $( \ell , d )$ -Feistel with $2 ^ { D }$ chosen plaintexts, we use $( k _ { 1 } , k _ { 2 } )$ satisfying 

$$
(k _ {1}, k _ {2}) = \left\{ \begin{array}{l l} (D - \ell , \ell) & \text { for } \ell \leq D, \\ (0, D) & \text { for } D <   \ell . \end{array} \right.
$$


Algorithm 1 Path search algorithm for integral distinguishers on $( \ell , d ) .$ -Feistel


1: procedure FeistelFuncEval( $\ell,d,k_{1},k_{2}$ )
2: $q\Leftarrow0$ 3:    for X=0 to $k_{1}$ do
4: $L\Leftarrow k_{2}+\lceil X/d\rceil$ 5:    if $L\leq\ell$ then
6: $q\Leftarrow q+1$ 7: $\boldsymbol{k}^{(q)}\Leftarrow(L,k_{1}-X)$ 8:    end if
9:    end for
10:    return $\boldsymbol{k}^{(1)},\ldots,\boldsymbol{k}^{(q)}$ 11: end procedure

12: procedure IntegralPathSearch( $\ell,d,r=0,k_{1},k_{2}$ )
13: $\boldsymbol{k}^{(1)},\ldots,\boldsymbol{k}^{(q)}\Leftarrow\text{FeistelFuncEval}(\ell,d,k_{1},k_{2})$ 14: $D\Leftarrow\max\{k_{1}^{(1)}+k_{2}^{(1)},k_{1}^{(2)}+k_{2}^{(2)},\ldots,k_{1}^{(q)}+k_{2}^{(q)}\}$ 15:    while 1<D do
16: $r\Leftarrow r+1$ 17:    for i=1 to q do
18: $\boldsymbol{k}^{(i,1)},\ldots,\boldsymbol{k}^{(i,p_{i})}\Leftarrow\text{FeistelFuncEval}(\ell,d,k_{1}^{(i)},k_{2}^{(i)})$ 19:    end for
20: $(\boldsymbol{k}^{(1)},\boldsymbol{k}^{(2)},\ldots,\boldsymbol{k}^{(q^{\prime})})\Leftarrow\text{SizeReduce}(\boldsymbol{k}^{(1,1)},\boldsymbol{k}^{(1,2)},\ldots,\boldsymbol{k}^{(q,p_{q})})$ 21: $D\Leftarrow\max\{k_{1}^{(1)}+k_{2}^{(1)},k_{1}^{(2)}+k_{2}^{(2)},\ldots,k_{1}^{(q^{\prime})}+k_{2}^{(q^{\prime})}\}$ 22: $q\Leftarrow q^{\prime}$ 23:    end while
24:    return r
25: end procedure 

For the comparison with our integral distinguishers, we consider two previous methods, one is the propagation characteristic of the integral property and another is the estimation of the algebraic degree. We first consider the propagation characteristic of the integral property. If the F -function is a non-bijective function, the propagation characteristic does not construct sufficient distinguishers. Therefore, results introduced by the integral property are only shown when the F -function is bijective. We next consider the estimation of the algebraic degree. Unfortunately, since we do not know the improved bound against the Feistel Network, we use the trivial bound for the Feistel Network. Assume that the left half of the plaintext is constant. For any r-round (`, d)-Feistel, it can be observed that the function, which associates the right half of the ciphertext with the right half of the plaintext, has degree at most $d ^ { r - 2 }$ for $2 \ \leq \ r$ . Therefore, we can construct the r-round integral distinguishers with $2 ^ { d ^ { r - 2 } + 1 }$ chosen plaintexts. Since the right half of the plaintext is at most ` bits, the distinguisher can be constructed with $2 ^ { d ^ { r - 2 } + 1 } < 2 ^ { \ell }$ . 

As a result, as far as we try, all distinguishers constructed by the division property are “better” than those by previous methods. We summarize integral distinguishers on other (`, d)-Feistel in Appendix B. We already know a “better” integral distinguisher on Camellia in [YPK02], but it is constructed by using the specific feature of Camellia. On the other hand, our method is generic distinguishing attacks against (`, d)-Feistel. From the result of (64, 7)-Feistel, it shows that even if the F -function of Camellia is chosen from any functions with degree 7, the modified Camellia has the 6-round integral distinguisher. 

Integral Distinguishers on Simon Family Although our attack is a generic attack, it can create new integral distinguishers on the Simon family [BSS+13]. Simon is a lightweight block ciphers proposed by the National Security Agency. Since Simon has a non-bijective F -function and a bit-oriented structure, it is complicated task to construct the integral distinguisher. The division property theoretically shows that Simon 32, 48, 64, 96, and 128 have at least 9-, 11-, 11-, 13-, and 13-round integral distinguishers, respectively. Table 3 shows the comparison between our distinguishers and previous ones by the degree estimation. On the other hand, Wang et al. showed that Simon 32 has the 15-round integral distinguisher by experiments [WLV+14]. Therefore, there are 6-round differences between our theoretical result and Wang’s experimental result. Our distinguisher is valid against all (32, 2)-Feistel and it does not exploit the feature of the round function. Namely, we expect that the 6-round difference is derived from the specification of the round function of Simon 32. 


Table 2. The number of chosen plaintexts to construct r-round integral distinguishers on (32, 5)- and (64, 7)-Feistel. Our distinguishers are got by implementing Algorithm 1.


<table><tr><td rowspan="2">Target [Application]</td><td rowspan="2">F-function</td><td colspan="6"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td></tr><tr><td rowspan="2">(32,5)-Feistel [DES]</td><td rowspan="2">non-bijection</td><td>26</td><td>51</td><td>62</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>26</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="3">(64,7)-Feistel [Camellia]</td><td rowspan="3">bijection</td><td>50</td><td>98</td><td>124</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>50</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td>64</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>integral</td><td>KW02</td></tr></table>


Table 3. The number of chosen plaintexts to construct r-round integral distinguishers on the Simon family, where the F -function is not bijective. Our distinguishers are got by implementing Algorithm 1.


<table><tr><td rowspan="2">Target [Application]</td><td colspan="8"><eq>\log_2(\#texts)</eq></td><td>Method</td><td rowspan="2">Reference</td></tr><tr><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td><td>r=12</td><td>r=13</td><td></td></tr><tr><td rowspan="2">(16,2)-Feistel [SIMON 32]</td><td>17</td><td>25</td><td>29</td><td>31</td><td>-</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>[Knu94 BC13]</td></tr><tr><td rowspan="2">(24,2)-Feistel [SIMON 48]</td><td>17</td><td>29</td><td>39</td><td>44</td><td>46</td><td>47</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="2">(32,2)-Feistel [SIMON 64]</td><td>17</td><td>33</td><td>49</td><td>57</td><td>61</td><td>63</td><td>-</td><td>-</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>[Knu94 BC13]</td></tr><tr><td rowspan="2">(48,2)-Feistel [SIMON 96]</td><td>17</td><td>33</td><td>57</td><td>77</td><td>87</td><td>92</td><td>94</td><td>95</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>33</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr><tr><td rowspan="2">(64,2)-Feistel [SIMON 128]</td><td>17</td><td>33</td><td>65</td><td>97</td><td>113</td><td>121</td><td>125</td><td>127</td><td>our</td><td>Sect. 4.3</td></tr><tr><td>17</td><td>33</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>Knu94 BC13</td></tr></table>

## 5 Improved Integral Distinguishers on Substitute-Permutation Network

## 5.1 Substitute-Permutation Network

(`, d, m)-SPN The Substitute-Permutation Network (SPN) is another important structure for block ciphers. The SPN has a round function that consists of an S-Layer and a P-Layer, and a block cipher is designed by iterating the round function. We now define an $( \ell , d , m ) { \cdot } \mathrm { S P N }$ , whose round function has m `-bit S-boxes in the S-Layer and one (`m)-bit linear function in the P-Layer. Here, each S-box is any bijective function whose degree is at most $d ,$ and an (`m)-bit linear function is any bijective function whose degree is at most 1. Figure 6 shows the round function of the SPN. Nowadays, many block ciphers adopting (`, d, m)-SPN have been proposed, e.g. AES [U.S01], PRESENT [BKL+07], and Serpent [ABK98] adopt (8, 7, 16)-, (4, 3, 16)-, and (4, 3, 32)-SPN, respectively. Moreover, Keccak-f [DBPA11], which is a permutation in the hash function Keccak, can be regarded as (5, 2, 320)-SPN. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/b915d7c9-7991-42a5-85b0-61d9f2301575/af00b0f1edbe2d5435ae0f09c154da1b81cfa3910431d6e6aadcc74fea8f4801.jpg)



Fig. 6. (`, d, m)-SPN


## 5.2 Propagation Characteristic for SPN

This section shows that the division property is useful to construct integral distinguishers on $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ . We first prepare the set of the input of the S-Layer such that $k _ { i }$ bits of the input of the i-th S-box are active and the rest $( \ell - k _ { i } )$ bits are constant. In this case, the input set has the division property $\mathcal { D } _ { k } ^ { \ell , m }$ . We first evaluate the propagation characteristic against the S-Layer. Next, the P-Layer is applied but the input and output take a value of $\mathbb { F } _ { 2 } ^ { \ell m }$ $\mathcal { D } _ { k } ^ { \ell , m }$ $\mathcal { D } _ { k } ^ { \ell m }$ the propagation characteristic against the P-Layer. Since the S-Layer is applied again after $\mathcal { D } _ { k } ^ { \ell m }$ $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { \ell , m }$ D`,mk(1),k(2),...,k(q) . After the second round, we evaluate the propagation characteristic of this collective division property. 

- S-Layer Assume that the input set of the S-Layer has the division property $\mathcal { D } _ { \boldsymbol { k } } ^ { \ell , m }$ . Since the S-Layer consists of m `-bit S-boxes with degree $d ,$ the output set of the S-Layer has $\mathcal { D } _ { k ^ { \prime } } ^ { \ell , m }$ $k _ { i } < \ell$ $k _ { i } ^ { \prime }$ $k _ { i } ^ { \prime } = \lceil k _ { i } / d \rceil . \mathrm { I f } k _ { i } = \ell \mathrm { h o l d s } , k _ { i } ^ { \prime }$ as $k _ { i } ^ { \prime } = \ell .$ . 

- Concatenation (Conversion form S-Layer to P-Layer) The output of the S-Layer is expressed in a value of $( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m }$ , but the input of the P-Layer is expressed in a value of $\mathbb { F } _ { 2 } ^ { \ell m }$ . Let $\mathbb { X }$ be the output set of the S-Layer whose elements take a value of $( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m }$ . Let $\mathbb { Y }$ be the input set of the P-Layer whose elements take a value of $\mathbb { F } _ { 2 } ^ { \ell m }$ . The transformation is generally implemented by a simple bit concatenation, namely, $y = ( x _ { 1 } \| x _ { 2 } \| \cdot \cdot \cdot \| x _ { m } )$ where $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ and $y$ are values of X and $\mathbb { Y } ,$ , respectively. We now consider the conversion of the division property from D`,mk $\mathcal { D } _ { k } ^ { \ell , m }$ $\mathcal { D } _ { k ^ { \prime } } ^ { \ell m }$ $\pi _ { v } ( y )$ $y \in \mathbb { Y }$ $\begin{array} { r } { w _ { v } \geq \sum _ { i = 1 } ^ { m } k _ { i } } \end{array}$ $\mathcal { D } _ { k ^ { \prime } } ^ { \ell m }$ $\textstyle k ^ { \prime } = \sum _ { i = 1 } ^ { m } k _ { i }$ 

- P-Layer The P-Layer consists of an (`m)-bit linear function. Since the degree of the linear function is at most 1, there is no change in the division property. 

- Partition (Conversion form P-Layer to S-Layer) The output of the P-Layer is expressed in a value of $\mathbb { F } _ { 2 } ^ { \ell m }$ , but the input of the S-Layer is expressed in a value of $( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m }$ . Let X be the output set of the P-Layer whose elements take a value of $\mathbb { F } _ { 2 } ^ { \ell m }$ . Let Y be the input set of the S-Layer whose elements take a value of $( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m }$ . The transformation is 


Algorithm 2 Path search algorithm for integral distinguishers on (`, d, m)-SPN


1: procedure IntegralPathSearch( $\ell, d, m, r = 0, k_{1}, k_{2}, \ldots, k_{m}$ )
2: if $k_{i} < \ell$ then $k_{i} \Leftarrow \lceil k_{i}/d \rceil$ ▷ 1-st round S-Layer
3: end if
4: $k \Leftarrow \sum_{i=1}^{m} k_{i}$ ▷ 1-st round Concatenation and P-Layer
5: while 1 < k do
6: $r \Leftarrow r + 1$ 7: if $k \leq (\ell - 1)m$ then $k \Leftarrow \lceil k/d \rceil$ ▷ (r + 1)-th round
8: else $k \Leftarrow \lceil \frac{\ell - 1}{d} \rceil (\ell m - k) + \ell (m - \ell m + k)$ ▷ (r + 1)-th round
9: end if
10: end while
11: return r
12: end procedure 

generally implemented by a simple bit partition, namely, $( y _ { 1 } \| y _ { 2 } \| \cdot \cdot \cdot \| y _ { m } ) = x$ where x and $\left( y _ { 1 } , y _ { 2 } , \ldots , y _ { m } \right)$ are values of X and $\mathbb { Y } ,$ respectively. We now consider the conversion $\mathcal { D } _ { k } ^ { \ell m }$ $\mathcal { D } _ { k ^ { \prime } } ^ { \ell , m }$ $\mathcal { D } _ { k } ^ { \ell m }$ , the sufficient condition that the parity of $\pi _ { u } ( x )$ for all $x \in \mathbb { X }$ becomes unknown is $k \leq w _ { u }$ . Therefore, the input set of the S-Layer has the collective division property `,m $\mathcal { D } _ { { \pmb k } ^ { \prime ( 1 ) } , { \pmb k } ^ { \prime ( 2 ) } , \ldots , { \pmb k } ^ { \prime ( q ) } } ^ { \ell , m }$ Dk0(1),k0(2),...,k0(q) , where q denotes the number of all possible vectors satisfying k0(j)1 + $k _ { 1 } ^ { \prime \left( j \right) } +$ $k _ { 2 } ^ { \prime ( j ) } + \cdot \cdot \cdot + k _ { m } ^ { \prime ( j ) } = k ( 1 \leq j \leq q )$ characteristic of the collective division property. 

We can construct the integral distinguisher by evaluating the propagation characteristic of the collective division property. However, since the size of $q$ extremely expands, it is infeasible to execute the straightforward implementation. Therefore, we show more efficient technique. Let X be the input set of the S-Layer, and the elements take a value of $( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m }$ . $\mathcal { D } _ { \pmb { k } ^ { ( 1 ) } , \pmb { k } ^ { ( 2 ) } , \dots , \pmb { k } ^ { ( q ) } } ^ { \ell , m }$ D`,mk(1),k(2),...,k(q) that is created by the partition of the division property $\mathcal { D } _ { k } ^ { \ell m }$ . I f $k > ( \ell - 1 ) m$ holds, at least $( m - \ell m + k )$ elements of $\pmb { k } ^ { ( j ) }$ have to become `. In this case, the rest elements have to become ` − 1. Since the S-Layer derives $\lceil \frac { \ell - 1 } { d } \rceil$  `− 1  and ` from $( \ell - 1 )$ and $\ell ,$ respectively, the output set has the division $\mathcal { D } _ { k ^ { \prime } } ^ { \ell m }$ $k ^ { \prime }$ 

$$
k ^ {\prime} = \left\{ \begin{array}{l l} \big \lceil \frac {\ell - 1}{d} \big \rceil (\ell m - k) + \ell (m - \ell m + k) & \text { for } k > (\ell - 1) m, \\ \big \lceil \frac {k}{d} \big \rceil & \text { for } k \leq (\ell - 1) m. \end{array} \right.
$$

Here, if $k \le ( \ell - 1 )$ )m holds, we simply regard the round function of $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ as one (`m)-bit S-box with degree d. 

## 5.3 Path Search Algorithm for (`, d, m)-SPN

We now consider integral distinguishers on $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ . We first prepare the set of chosen plaintexts such that $k _ { i }$ bits of the input of the i-th S-box are active and the rest $( \ell - k _ { i } )$ bits are constant. Namely, we prepare $2 \Sigma _ { i = 1 } ^ { m } k _ { i }$ chosen plaintexts. The input set has the division property $\mathcal { D } _ { k } ^ { \ell , m }$ . Algorithm $\bigstar$ shows the path search algorithm to construct the integral distinguisher. 

Results Table 4 shows the number of required chosen plaintexts to construct the r-round integral distinguisher on (4, 3, 16)- and (8, 7, 16)-SPN, where PRESENT [BKL+07] and 


Table 4. The number of chosen plaintexts to construct r-round integral distinguishers on (`, d, m)-SPN. Our distinguishers are got by implementing Algorithm 2.


<table><tr><td rowspan="2">Target</td><td colspan="5"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td></tr><tr><td rowspan="2">(4,3,16)-SPN [PRESENT]</td><td>12</td><td>28</td><td>52</td><td>60</td><td>-</td><td>our</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>52</td><td>60</td><td>63</td><td>-</td><td>degree</td><td>BCC11</td></tr><tr><td rowspan="2">(8,7,16)-SPN [AES]</td><td>56</td><td>120</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 5.3</td></tr><tr><td>117</td><td>127</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>BCC11</td></tr></table>


Table 5. The number of chosen plaintexts to construct r-round integral distinguishers on Keccak-f and Serpent. Our distinguishers are got by implementing Algorithm 2.


<table><tr><td rowspan="2">Target [Application]</td><td colspan="8"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9&#x27;</td><td>r=10</td></tr><tr><td rowspan="2">(4,3,32)-SPN [Serpent]</td><td>12</td><td>28</td><td>84</td><td>113</td><td>124</td><td>-</td><td>-</td><td>-</td><td>our</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>82</td><td>113</td><td>123</td><td>127</td><td>-</td><td>-</td><td>-</td><td>degree</td><td>BCC11</td></tr><tr><td rowspan="2">Target [Application]</td><td colspan="8"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td><td>r=12</td><td>r=13</td><td>r=14</td><td>r=15</td></tr><tr><td rowspan="2">(5,2,320)-SPN [KECCAK-f]</td><td>130</td><td>258</td><td>515</td><td>1025</td><td>1410</td><td>1538</td><td>1580</td><td>1595</td><td>our</td><td>Sect. 5.3</td></tr><tr><td>257</td><td>513</td><td>1025</td><td>1409</td><td>1537</td><td>1579</td><td>1593</td><td>1598</td><td>degree</td><td>BCC11</td></tr></table>

AES [U.S01] are classified into (4, 3, 16)- and (8, 7, 16)-SPN, respectively. When we construct the integral distinguisher on (`, d, m)-SPN with $2 ^ { \overset { \prime } { D } }$ chosen plaintexts, we use a vector k satisfying 

$$
k _ {i} = \left\{ \begin{array}{l l} \ell & \text {for i\ell\leq D}, \\ D - (i - 1) \ell & \text {for (i - 1)\ell\leq D <   i\ell}, \\ 0 & \text {for D <   (i - 1)\ell}. \end{array} \right.
$$

For the comparison with our integral distinguishers, we first consider the propagation characteristic of the integral property. However, it does not construct a sufficient distinguisher because the P-Layer is any linear function. Next, we estimate the algebraic degree by using the method proposed by Boura et al. We show the method in Appendix A. 

As a result, as far as we try, all distinguishers constructed by the division property are $\mathrm { ^ { 6 6 } b e t t e r } ^ { 3 }$ than those by previous methods. We summarize integral distinguishers on other $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ in Appendix C. We already know the 7-round integral distinguisher on PRESENT in [WW13] and the 4-round integral distinguisher on AES in [KW02]. However, they are constructed by using the specific feature of each block cipher. On the other hand, our method is generic distinguishing attacks against $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ . From the result of (4, 3, 16)-SPN, it shows that even if the P-Layer of PRESENT is chosen from any bijective linear functions, the modified PRESENT has the 6-round integral distinguisher. Similarly, from the result of (8, 7, 16)-SPN, it shows that even if the P-Layer of AES is chosen from any bijective linear function, the modified AES still has the 4-round integral distinguisher. 

Integral Distinguishers on Serpent and Keccak-f Although our attack is a generic attack, it can create new integral distinguishers on Serpent and Keccak-f. Serpent is one of AES finalists and is classified into (4, 3, 32)-SPN. The existing integral distinguisher is shown in [ZRHD08], and it shows that Serpent has 3.5-round integral distinguisher. On the other hand, we show that all (4, 3, 32)-SPNs have at least 7-round integral distinguishers with $2 ^ { 1 2 4 }$ chosen plaintexts. Table 5 shows the comparison between our distinguishers and previous ones by the degree estimation. 

Keccak is chosen as SHA-3, and the core function Keccak-f is classified into (5, 2, 320)- SPN. Boura et al. estimated the algebraic degree of Keccak-f in [BCC11]. We search for the integral distinguisher by using Algorithm 2. As a result, our distinguishers can reduce the number of chosen plaintexts compared with previous ones. Table 5 shows the comparison between our distinguishers and previous ones. 

## 6 Toward Dedicated Attack

We introduced the division property in Sect. 3, and proposed distinguishing attacks against the Feistel Network and the SPN in Sect. 4 and Sect. 5, respectively. In this section, we show that the division property is also useful to construct the dedicated attack against specific ciphers. As an example, we show integral distinguishers on AES-like ciphers. 

## 6.1 AES-Like Cipher

(`, d, m)-AES AES is a 128-bit block cipher, and an intermediate text of AES is expressed in a 4×4 matrix whose elements are 8 bits. The round function of AES consists of SubBytes, ShiftRows, MixColumns, and AddRoundKey, where each function is defined as follows: 

– SubBytes (SB) : It substitutes each byte in the matrix into another byte by an S-box. 

– ShiftRows (SR) : Each byte of the i-th row is rotated i  1 bytes to the left. 

– MixColumns (MC) : It diffuses bytes within each column by a linear function. 

– AddRoundKey (AK) : A round key is XORed with the intermediate text. 

We define an (`, d, m)-AES, where `, d, and m denote the bit length of an S-box, the algebraic degree of an S-box, and the size of the matrix, respectively. This intermediate text is expressed in an m × m matrix whose elements are ` bits. Let $\pmb { X } \in ( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m \times m }$ be an input of the round function, which is arranged as 

$$
\left[ \begin{array}{c c c c} x _ {1, 1} & x _ {1, 2} & \dots & x _ {1, m} \\ x _ {2, 1} & x _ {2, 2} & \dots & x _ {2, m} \\ \vdots & \vdots & \ddots & \vdots \\ x _ {m, 1} & x _ {m, 2} & \dots & x _ {m, m} \end{array} \right].
$$

Let $\pmb { Y } \in ( \mathbb { F } _ { 2 } ^ { \ell } ) ^ { m \times m }$ be an output of the round function, which is calculated as $\pmb { Y } = ( \mathrm { A K } \circ$ $\operatorname { M C } \circ \operatorname { S R } \circ \operatorname { S B } ) ( X )$ . Each function is the same as that of AES except for the scale. For instance, AES [U.S01] and LED [GPPR11] adopt $( 8 , 7 , 4 ) – \mathrm { A E S }$ and (4, 3, 4)-AES, respectively. Moreover, $P _ { 2 5 6 }$ of PHOTON [GPP11] adopts (4, 3, 8)-AES2. 

## 6.2 Path Search Algorithm for (`, d, m)-AES

Section 5 shows how to construct integral distinguishers on $( \ell , d , m ) { \mathrel { \operatorname { \cdot } } } \mathrm { S P N }$ , but practical block ciphers have a specific P-Layer. For instance, the P-Layer in AES consists of ShiftRows and MixColumns, and it is not any linear function. Taking into account the structure of the P-Layer, we can construct more effective algorithm. In this section, as an example, we show a path search algorithm to construct integral distinguishers on $( \ell , d , m ) { \mathrm { - A E S } }$ . 


Algorithm 3 Evaluating algorithm against the round function of $( \ell , d , m ) { \mathrm { - A E S } }$


1: procedure AesFuncEval( $\ell, d, m, K$ )
2:    for r = 1 to m do
3:    for c = 1 to m do
4:    if $k_{r,c} < \ell$ then $k_{r,c} \Leftarrow \lceil k_{r,c}/d \rceil$ ▷ SubBytes
5:    end if
6:    end for
7:    end for
8: $K \Leftarrow \text{ShiftRows}(K)$ ▷ ShiftRows
9: $k'_{c} \Leftarrow \sum_{r=1}^{m} k'_{r,c}$ for all c ▷ MixColumns
10: $k' \Leftarrow \text{sort}(k')$ 11:    return $k'$ 12: end procedure 


Algorithm 4 Path search algorithm for integral distinguishers on $( \ell , d , m ) { \mathrm { - A E S } }$


1: procedure IntegralPathSearch( $\ell, d, m, r = 0, K \in \{0, 1, \ldots, \ell\}^{m \times m}$ )
2: $k^{(1)} \Leftarrow \text{AesFuncEval}(\ell, d, m, K)$ ▷ 1-st round
3: $D \Leftarrow \sum_{c=1}^{m} k_{c}^{(1)}$ 4: $q \Leftarrow 1$ 5: while 1 < D do
6: $r \Leftarrow r + 1$ 7:    for i = 1 to q do
8: $K^{(i,1)}, \ldots, K^{(i,s)} \Leftarrow \text{Partition}(k^{(i)})$ 9:    for j = 1 to s do
10: $\bar{k}^{(1)}, \ldots, \bar{k}^{(t)} \Leftarrow \text{AesFuncEval}(\ell, d, m, K^{(i,j)})$ ▷ (r + 1)-th round
11:    if (i, j) = (1, 1) then
12: $k'^{(1)}, \ldots, k'^{(q')} \Leftarrow \text{SizeReduce}(\bar{k}^{(1)}, \ldots, \bar{k}^{(t)})$ 13:    else
14: $k'^{(1)}, \ldots, k'^{(q'')} \Leftarrow \text{SizeReduce}(k'^{(1)}, \ldots, k'^{(q')} , \bar{k}^{(1)}, \ldots, \bar{k}^{(t)})$ 15: $q' \Leftarrow q''$ 16:    end if
17:    end for
18:    end for
19: $k^{(i)} \Leftarrow k'^{(i)}$ for all $1 \leq i \leq q'$ 20: $q \Leftarrow q'$ 21: $D \Leftarrow \max\{\sum_{c=1}^{m} k_{c}^{(1)}, \sum_{c=1}^{m} k_{c}^{(2)}, \ldots, \sum_{c=1}^{m} k_{c}^{(q)}\}$ 22:    end while
23:    return r
24: end procedure 

Algorithm 3 evaluates the propagation characteristic of the division property against the round function of AES-like ciphers, and it calls ShiftRows and sort. ShiftRows performs a similar transformation to SR. sort is the sorting algorithm, which is useful for feasible implementation. Algorithm 4 shows the path search algorithm, and it calls Partition, AesFuncEval, and SizeReduce. Partition $( \pmb { k } ^ { ( i ) } )$ calculates all possible $\pmb { K } ^ { ( i , j ) }$ satisfying 

$$
\left(\sum_ {r = 1} ^ {m} k _ {r, 1} ^ {(i, j)}, \sum_ {r = 1} ^ {m} k _ {r, 2} ^ {(i, j)}, \dots , \sum_ {r = 1} ^ {m} k _ {r, m} ^ {(i, j)}\right) = (k _ {1} ^ {(i)}, k _ {2} ^ {(i)}, \dots , k _ {m} ^ {(i)}),
$$

where $\mathbb { S } _ { k ^ { ( i , j ) } } ^ { \ell m , m } \subseteq \mathbb { S } _ { k ^ { ( i ^ { \prime } , j ^ { \prime } ) } } ^ { \ell m , m }$ $0 \leq k _ { r , c } ^ { ( i , j ) } \leq \ell$ . holds. SizeReduce eliminates $\boldsymbol { k } ^ { ( i , j ) }$ if there exists $( i ^ { \prime } , j ^ { \prime } )$ satisfying 

Notice that the size of $q$ in the division property extremely expands when the partition of the division property is executed (see the 8-th line in Algorithm 4). Namely, our algorithm takes large execution time and large memory capacity if we straightforwardly implement our algorithm. Therefore, we use an effective method, which uses the feature of $( \ell , d , m ) { \cdot } \mathrm { A E S } .$ , for the feasible implementation. Notice that each column of $( \ell , d , m ) { \mathrm { - A E S } }$ is equivalent each $\mathcal { D } _ { \boldsymbol { k } , \boldsymbol { k } ^ { \prime } } ^ { \ell m , m }$ $\pmb { k } ^ { \prime }$ $k ,$ the division property of the next round calculated from k is exactly the same as that from $\pmb { k } ^ { \prime }$ because columns of $( \ell , d , m ) { \mathrm { - A E S } }$ are equivalent each other. Namely, it is enough to save either, and we implement it by a sorting algorithm (see the 10-th line in Algorithm 3). This technique enables us to execute our path search algorithm feasibly in many parameters. 


Table 6. The number of chosen plaintexts to construct r-round integral distinguishers on $( 4 , 3 , m ) \mathrm { - A E S }$ . Our distinguishers are got by implementing Algorithm 2 and Algorithm 4.


<table><tr><td rowspan="2">Target [Application]</td><td colspan="6"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Method</td><td rowspan="2">Reference</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td></tr><tr><td rowspan="4">(4,3,4)-AES [LED]</td><td>4</td><td>12</td><td>32</td><td>52</td><td>-</td><td>-</td><td>our (AES)</td><td>Sect. 6.2</td></tr><tr><td>12</td><td>28</td><td>52</td><td>60</td><td>-</td><td>-</td><td>our (SPN)</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>52</td><td>60</td><td>63</td><td>-</td><td>-</td><td>degree</td><td>BCC11</td></tr><tr><td>4</td><td>16</td><td>-</td><td>-</td><td>-</td><td>-</td><td>integral</td><td>DKR97 KW02</td></tr><tr><td rowspan="4">(4,3,5)-AES [P100 in PHOTON]</td><td>4</td><td>12</td><td>20</td><td>72</td><td>97</td><td>-</td><td>our (AES)</td><td>Sect. 6.2</td></tr><tr><td>12</td><td>28</td><td>76</td><td>92</td><td>-</td><td>-</td><td>our (SPN)</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>76</td><td>92</td><td>98</td><td>-</td><td>-</td><td>degree</td><td>BCC11</td></tr><tr><td>4</td><td>20</td><td>-</td><td>-</td><td>-</td><td>-</td><td>integral</td><td>DKR97 KW02</td></tr><tr><td rowspan="4">(4,3,6)-AES [P144 in PHOTON]</td><td>4</td><td>12</td><td>24</td><td>84</td><td>132</td><td>-</td><td>our (AES)</td><td>Sect. 6.2</td></tr><tr><td>12</td><td>28</td><td>84</td><td>124</td><td>140</td><td>-</td><td>our (SPN)</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>82</td><td>124</td><td>138</td><td>142</td><td>-</td><td>degree</td><td>BCC11</td></tr><tr><td>4</td><td>24</td><td>-</td><td>-</td><td>-</td><td>-</td><td>integral</td><td>DKR97 KW02</td></tr><tr><td rowspan="4">(4,3,7)-AES [P196 in PHOTON]</td><td>4</td><td>12</td><td>24</td><td>84</td><td>164</td><td>192</td><td>our (AES)</td><td>Sect. 6.2</td></tr><tr><td>12</td><td>28</td><td>84</td><td>160</td><td>184</td><td>192</td><td>our (SPN)</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>82</td><td>158</td><td>184</td><td>192</td><td>195</td><td>degree</td><td>BCC11</td></tr><tr><td>4</td><td>28</td><td>-</td><td>-</td><td>-</td><td>-</td><td>integral</td><td>DKR97 KW02</td></tr><tr><td rowspan="4">(4,3,8)-AES [P256 in PHOTON]</td><td>4</td><td>12</td><td>28</td><td>92</td><td>204</td><td>249</td><td>our (AES)</td><td>Sect. 6.2</td></tr><tr><td>12</td><td>28</td><td>84</td><td>200</td><td>237</td><td>252</td><td>our (SPN)</td><td>Sect. 5.3</td></tr><tr><td>28</td><td>82</td><td>198</td><td>237</td><td>250</td><td>254</td><td>degree</td><td>BCC11</td></tr><tr><td>4</td><td>32</td><td>-</td><td>-</td><td>-</td><td>-</td><td>integral</td><td>DKR97 KW02</td></tr></table>

Results Table 6 shows the number of required chosen plaintexts to construct r-round integral distinguishers on $( 4 , 3 , m ) { \mathrm { - A E S } }$ . When we construct the integral distinguisher on $( \ell , d , m ) { \mathrm { - A E S } }$ with $2 ^ { D }$ chosen plaintexts, we carefully choose the input matrix K. 

For the comparison with our improved integral distinguishers, we also show integral distinguishers by using the propagation characteristic of the integral property. We also estimate the algebraic degree by the method proposed Boura et al. (see Appendix A). Moreover, since $( 4 , 3 , m ) { \mathrm { - A E S } }$ are classified into $( 4 , 3 , m ^ { 2 } ) – \mathrm { S P N }$ , we construct integral distinguishers by Algorithm 2. 

As a result, as far as we try, all distinguishers constructed by the division property are at least better than those by previous methods. Especially, the advantage of our method is large when we construct the integral distinguisher with the small number of texts. For instance, our method shows that $( 4 , 3 , 8 ) { \mathrm { - A E S } }$ , which is adopted by $P _ { 2 5 6 }$ in PHOTON, has the 6-round distinguisher with $2 ^ { 9 2 }$ chosen plaintexts. If we regard (4, 3, 8)-AES as (4, 3, 64)-SPN, $2 ^ { 2 0 0 }$ chosen plaintexts are required to construct the distinguisher. 

## 7 Conclusions

In this paper, we proposed the fundamental technique to improve integral distinguishers, and showed structural cryptanalyses against the Feistel Network and the SPN. Our new technique uses the division property, which is the generalization of the integral property. It can effectively construct integral distinguishers even if block ciphers have non-bijective functions, bit-oriented structures, and low-degree functions. For the Feistel Network, when the algebraic degree of the F -function is smaller than the bit length of the F -function, our method can attack more rounds than previous generic attacks. Moreover, we theoretically showed that Simon 48, 64, 96, and 128 have 11-, 11-, 13-, and 13-round integral distinguishers, respectively. For the SPN, our method extremely reduces the required number of chosen plaintexts compared with previous methods. Moreover, we improved integral distinguishers on Keccak-f and Serpent. The division property is useful to construct integral distinguishers against specific ciphers. As one example, we showed a path search algorithm to construct integral distinguishers on the AES-like cipher, which is the sub class of the SPN. From this fact, we expect that the division property can construct many improved integral distinguishers against specific ciphers by constructing the dedicated path search algorithm. 



DEMS14. Christoph Dobraunig, Maria Eichlseder, Florian Mendel, and Martin Schl¨affer. Ascon v1, 2014. Submission to CAESAR competition. 



Acknowledgments. The authors would like to thank Deukjo Hong for his helpful pointing out. 



DKR97. Joan Daemen, Lars R. Knudsen, and Vincent Rijmen. The block cipher Square. In FSE, volume 1267 of LNCS, pages 149–165, 1997. 



## References



DPAR00. Joan Daemen, Micha¨el Peeters, Gilles Van Assche, and Vincent Rijmen. The Noekeon block cipher., 2000. submitted to the NESSIE project, available at http://gro.noekeon.org/. 





DR02. Joan Daemen and Vincent Rijmen. The Design of Rijndael: AES - The Advanced Encryption Standard. Information Security and Cryptography. Springer, 2002. 





ABB+14. Elena Andreeva, Beg¨ul Bilgin, Andrey Bogdanov, Atul Luykx, Florian Mendel, Bart Mennink, Nicky Mouha, Qingju Wang, and Kan Yasuda. PRIMATEs v1.02, 2014. Submission to CAESAR competition. 





GPP11. Jian Guo, Thomas Peyrin, and Axel Poschmann. The PHOTON family of lightweight hash functions. In CRYPTO, volume 6841 of LNCS, pages 222–239, 2011. 





ABK98. Ross Anderson, Eli Biham, and Lars Knudsen. Serpent: A proposal for the Advanced Encryption Standard. NIST AES Proposal, 1998. 





GPPR11. Jian Guo, Thomas Peyrin, Axel Poschmann, and Matthew J. B. Robshaw. The LED block cipher. In CHES, volume 6917 of LNCS, pages 326–341, 2011. 





AIK+00. Kazumaro Aoki, Tetsuya Ichikawa, Masayuki Kanda, Mitsuru Matsui, Shiho Moriai, Junko Nakajima, and Toshio Tokita. Camellia: A 128-bit block cipher suitable for multiple platforms - design and analysis. In SAC, volume 2012 of LNCS, pages 39–56, 2000. 





IS13. Takanori Isobe and Kyoji Shibutani. Generic key recovery attack on Feistel scheme. In ASI-ACRYPT (1), volume 8269 of LNCS, pages 464–485. Springer, 2013. 





BC13. Christina Boura and Anne Canteaut. On the influence of the algebraic degree of $F ^ { - 1 }$ on the algebraic degree of G ◦ F . IEEE Transactions on Information Theory, 59(1):691–702, 2013. 





KLL+14. Elif Bilge Kavun, Martin Mehl Lauridsen, Gregor Leander, Christian Rechberger, Peter Schwabe, and Tolga Yal¸cin. Prøst v1.1, 2014. Submission to CAESAR competition. 





BCC11. Christina Boura, Anne Canteaut, and Christophe De Canni`ere. Higher-order differential properties of Keccak and Luffa. In FSE, volume 6733 of LNCS, pages 252–269, 2011. 





Knu94. Lars R. Knudsen. Truncated and higher order differentials. In FSE, volume 1008 of LNCS, pages 196–211, 1994. 





BKL+07. Andrey Bogdanov, Lars R. Knudsen, Gregor Leander, Christof Paar, Axel Poschmann, Matthew J. B. Robshaw, Yannick Seurin, and C. Vikkelsoe. PRESENT: an ultra-lightweight block cipher. In CHES, volume 4727 of LNCS, pages 450–466, 2007. 





Knu02. Lars R. Knudsen. The security of Feistel ciphers with six rounds or less. J. Cryptology, 15(3):207– 222, 2002. 





BR03. Paulo S. L. M. Barreto and Vincent Rijmen. The Whirlpool hashing function, 2003. submitted to the NESSIE project, available at http://www.larc.usp.br/~pbarreto/WhirlpoolPage.html. 





KW02. Lars R. Knudsen and David Wagner. Integral cryptanalysis (extended abstract). In FSE, volume 2365 of LNCS, pages 112–127, 2002. 





BS01. Alex Biryukov and Adi Shamir. Structural cryptanalysis of SASAS, 2001. 





Lai94. Xuejia Lai. Higher order derivatives and differential cryptanalysis. In Communications and Cryptography, volume 276 of The Springer International Series in Engineering and Computer Science, pages 227–233, 1994. 





BSS+13. Ray Beaulieu, Douglas Shors, Jason Smith, Stefan Treatman-Clark, Bryan Weeks, and Louis Wingers. The SIMON and SPECK families of lightweight block ciphers. IACR Cryptology ePrint Archive, 2013:404, 2013. 





LR88. Michael Luby and Charles Rackoff. How to construct pseudorandom permutations from pseudorandom functions. SIAM J. Comput., 17(2):373–386, 1988. 





CSW08. Christophe De Canni`ere, Hisayoshi Sato, and Dai Watanabe. Hash function Luffa - a SHA-3 candidate, 2008. Available at http://hitachi.com/rd/yrl/crypto/luffa/round1archive/ Luffa_Specification.pdf. 





LWZ11. Yanjun Li, Wenling Wu, and Lei Zhang. Improved integral attacks on reduced-round CLEFIA block cipher. In WISA, volume 7115 of LNCS, pages 28–39, 2011. 





CV02. Anne Canteaut and Marion Videau. Degree of composition of highly nonlinear functions and applications to higher order differential cryptanalysis. In EUROCRYPT, volume 2332 of LNCS, pages 518–533, 2002. 





MGH+14. Pawe? Morawiecki, Kris Gaj, Ekawat Homsirikamol, Krystian Matusiewicz, Josef Pieprzyk, Marcin Rogawski, Marian Srebrny, and Marcin Wøjcik. ICEPOLE v1, 2014. Submission to CAESAR competition. 





DBPA11. Joan Daemen, Guido Bertoni, Micha¨el Peeters, and Gilles Van Assche. The Keccak reference version 3.0, 2011. 





Pat04. Jacques Patarin. Security of random Feistel schemes with 5 or more rounds. In CRYPTO, volume 3152 of LNCS, pages 106–122, 2004. 





SK12. Naoki Shibayama and Toshinobu Kaneko. A peculiar higher order differential of CLEFIA. In ISITA, pages 526–530. IEEE, 2012. 





STA+14. Yu Sasaki, Yosuke Todo, Kazumaro Aoki, Yusuke Naito, Takeshi Sugawara, Yumiko Murakami, Mitsuru Matsui, and Shoichi Hirose. Minalpher v1, 2014. Submission to CAESAR competition. 





U.S77. U.S. DEPARTMENT OF COMMERCE/National Institute of Standards and Technology. DATA ENCRYPTION STANDARD (DES), 1977. Federal Information Processing Standards Publication 46. 





U.S01. U.S. DEPARTMENT OF COMMERCE/National Institute of Standards and Technology. Specification for the ADVANCED ENCRYPTION STANDARD (AES), 2001. Federal Information Processing Standards Publication 197. 





WLV+14. Qingju Wang, Zhiqiang Liu, Kerem Varici, Yu Sasaki, Vincent Rijmen, and Yosuke Todo. Cryptanalysis of reduced-round SIMON32 and SIMON48. In INDOCRYPT, volume 8885 of LNCS, pages 143–160. Springer, 2014. 





WW13. Shengbao Wu and Mingsheng Wang. Integral attacks on reduced-round PRESENT. In ICICS, volume 8233 of LNCS, pages 331–345, 2013. 





WZ11. Wenling Wu and Lei Zhang. LBlock: A lightweight block cipher. In ACNS, volume 6715 of LNCS, pages 327–344, 2011. 





YPK02. Yongjin Yeom, Sangwoo Park, and Iljun Kim. On the security of CAMELLIA against the Square attack. In FSE, volume 2365 of LNCS, pages 89–99, 2002. 





ZRHD08. Muhammad Reza Z’aba, H˚avard Raddum, Matthew Henricksen, and Ed Dawson. Bit-pattern based integral attack. In FSE, volume 5086 of LNCS, pages 363–381, 2008. 



## A Estimation of Algebraic Degree for (`, d, m)-SPN

If the degree of r iterated round functions is at most D, we can construct the r-round integral distinguisher with $2 ^ { D + 1 }$ chosen plaintexts. In a classical method, if the degree of the round function is at most $d ,$ the degree of r iterated round functions is bounded by $d ^ { r }$ . In 2011, Boura et al. showed tighter bound as follows. 

Theorem 1 ([BCC11]). Let S be a function from $\mathbb { F } _ { 2 } ^ { n }$ into $\mathbb { F } _ { 2 } ^ { n }$ corresponding to the concatenation of m smaller S-boxes, defined over $\mathbb { F } _ { 2 } ^ { n _ { 0 } }$ . Let $\delta _ { k }$ be the maximal degree $o f$ the product of any k bits of anyone of these S-boxes. Then, for any function G from Fn2 into $\mathbb { F } _ { 2 }$ , we have 

$$
\deg (G \circ S) \leq n - \frac {n - \deg (G)}{\gamma},
$$

where 

$$
\gamma = \max _ {1 \leq i \leq n _ {0} - 1} \frac {n _ {0} - i}{n _ {0} - \delta_ {i}}.
$$

By using this bound, we can estimate the degree of (`, d, m)-SPN. For instance, we show the degree of (4, 3, 64)-SPN as follows. 

<table><tr><td>Number of rounds</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td></tr><tr><td>Bound on degree</td><td>3</td><td>9</td><td>27</td><td>81</td><td>197</td><td>236</td><td>249</td><td>253</td><td>255</td></tr></table>

Therefore, we can construct the 8-round integral distinguisher on (4, 3, 64)-SPN with $2 ^ { 2 5 4 }$ chosen plaintexts. 

## B Integral Distinguishers on (`, d)-Feistel

Table 7 shows integral distinguishers on $( \ell , d )$ -Feistel, where $( \ell , d )$ -Feistel is defined in Sect. 4.1. If we construct the dedicated path search algorithm for the specific cipher, we expect that the algorithm can create better integral distinguishers. 


Table 7. The number of required chosen plaintexts to construct r-round integral distinguishers on $( \ell , d ) -$ Feistel. We get these values by implementing Algorithm 1.


<table><tr><td rowspan="2">Target</td><td rowspan="2">F-function</td><td colspan="9"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td><td>r=12</td><td>r=13</td><td>r=14</td></tr><tr><td rowspan="2">(16,2)</td><td>non-bijection</td><td>17</td><td>25</td><td>29</td><td>31</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>SIMON 32 [BSS+13]</td></tr><tr><td>bijection</td><td>16</td><td>23</td><td>28</td><td>30</td><td>31</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(24,2)</td><td>non-bijection</td><td>17</td><td>29</td><td>39</td><td>44</td><td>46</td><td>47</td><td>-</td><td>-</td><td>-</td><td>SIMON 48 [BSS+13]</td></tr><tr><td>bijection</td><td>17</td><td>27</td><td>38</td><td>43</td><td>46</td><td>47</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(32,2)</td><td>non-bijection</td><td>17</td><td>33</td><td>49</td><td>57</td><td>61</td><td>63</td><td>-</td><td>-</td><td>-</td><td>SIMON 64 [BSS+13]</td></tr><tr><td>bijection</td><td>17</td><td>32</td><td>47</td><td>56</td><td>60</td><td>62</td><td>63</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(48,2)</td><td>non-bijection</td><td>17</td><td>33</td><td>57</td><td>77</td><td>87</td><td>92</td><td>94</td><td>95</td><td>-</td><td>SIMON 96 [BSS+13]</td></tr><tr><td>bijection</td><td>17</td><td>33</td><td>55</td><td>76</td><td>86</td><td>91</td><td>94</td><td>95</td><td>-</td><td></td></tr><tr><td rowspan="2">(64,2)</td><td>non-bijection</td><td>17</td><td>33</td><td>65</td><td>97</td><td>113</td><td>121</td><td>125</td><td>127</td><td>-</td><td>SIMON 128 [BSS+13]</td></tr><tr><td>bijection</td><td>17</td><td>33</td><td>64</td><td>95</td><td>112</td><td>120</td><td>124</td><td>126</td><td>127</td><td></td></tr><tr><td rowspan="2">Target</td><td rowspan="2">F-function</td><td colspan="9"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td></tr><tr><td rowspan="2">(32,5)</td><td>non-bijection</td><td>6</td><td>26</td><td>51</td><td>62</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>DES [U.S77]</td></tr><tr><td>bijection</td><td>6</td><td>26</td><td>46</td><td>61</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(48,5)</td><td>non-bijection</td><td>6</td><td>26</td><td>64</td><td>90</td><td>95</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>6</td><td>26</td><td>59</td><td>89</td><td>95</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(64,5)</td><td>non-bijection</td><td>6</td><td>26</td><td>77</td><td>118</td><td>126</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>6</td><td>26</td><td>72</td><td>117</td><td>126</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">Target</td><td rowspan="2">F-function</td><td colspan="9"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td></tr><tr><td rowspan="2">(32,7)</td><td>non-bijection</td><td>8</td><td>35</td><td>60</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>8</td><td>32</td><td>59</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(48,7)</td><td>non-bijection</td><td>8</td><td>49</td><td>90</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>8</td><td>48</td><td>84</td><td>95</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(64,7)</td><td>non-bijection</td><td>8</td><td>50</td><td>104</td><td>125</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>8</td><td>50</td><td>98</td><td>124</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>Camellia [AIK+00]</td></tr><tr><td rowspan="2">Target</td><td>F-function</td><td colspan="9"><eq>\log_2(\#texts)</eq></td><td>Examples</td></tr><tr><td></td><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td><td></td></tr><tr><td rowspan="2">(32,31)</td><td>non-bijection</td><td>32</td><td>62</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>32</td><td>32</td><td>63</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(48,47)</td><td>non-bijection</td><td>48</td><td>94</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>48</td><td>48</td><td>95</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td rowspan="2">(64,63)</td><td>non-bijection</td><td>64</td><td>126</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>bijection</td><td>64</td><td>64</td><td>127</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(32,32)</td><td>non-bijection</td><td>33</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(48,48)</td><td>non-bijection</td><td>49</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(64,64)</td><td>non-bijection</td><td>65</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr></table>

## C Integral Distinguishers on $( \ell , d , m ) { \cdot } \mathbf { S } \mathbf { P } \mathbf { N }$

Table 8 shows integral distinguishers on $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ , where $( \ell , d , m ) { \mathrel { - } } \mathrm { S P N }$ is defined in Sect. 5.1. If we construct the dedicated path search algorithm for the specific cipher, we expect that the algorithm can create better integral distinguishers. 


Table 8. The number of required chosen plaintexts to construct r-round integral distinguishers on $( \ell , d , m ) \cdot$ - SPN. We get these values by implementing Algorithm 2.


<table><tr><td rowspan="2">Target</td><td rowspan="2">Size(bits)</td><td colspan="7"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td></tr><tr><td>(4,3,16)</td><td>64</td><td>28</td><td>52</td><td>60</td><td>-</td><td>-</td><td>-</td><td>-</td><td>PRESENT <eq>[BKL^+07]</eq>, LED <eq>[GPPR11]</eq></td></tr><tr><td>(4,3,24)</td><td>96</td><td>28</td><td>76</td><td>89</td><td>-</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(4,3,32)</td><td>128</td><td>28</td><td>84</td><td>113</td><td>124</td><td>-</td><td>-</td><td>-</td><td>Serpent <eq>[ABK98]</eq>, NOEKEON <eq>[DPAR00]</eq></td></tr><tr><td>(4,3,40)</td><td>160</td><td>28</td><td>84</td><td>136</td><td>152</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(4,3,48)</td><td>192</td><td>28</td><td>84</td><td>156</td><td>180</td><td>188</td><td>-</td><td>-</td><td></td></tr><tr><td>(4,3,56)</td><td>224</td><td>28</td><td>84</td><td>177</td><td>209</td><td>220</td><td>-</td><td>-</td><td></td></tr><tr><td>(4,3,64)</td><td>256</td><td>28</td><td>84</td><td>200</td><td>237</td><td>252</td><td>-</td><td>-</td><td>Prøst-128 <eq>[KLL^+14]</eq>, Minalpher-<eq>P</eq> <eq>[STA^+14]</eq></td></tr><tr><td>(4,3,128)</td><td>512</td><td>28</td><td>84</td><td>244</td><td>424</td><td>484</td><td>504</td><td>509</td><td>Prøst-256 <eq>[KLL^+14]</eq></td></tr><tr><td rowspan="2">Target</td><td rowspan="2">Size(bits)</td><td colspan="7"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td>r=10</td><td>r=11</td></tr><tr><td>(5,2,40)</td><td>200</td><td>18</td><td>35</td><td>65</td><td>130</td><td>178</td><td>195</td><td>-</td><td>PRIMATE-80 <eq>[ABB^+14]</eq></td></tr><tr><td>(5,2,56)</td><td>280</td><td>18</td><td>35</td><td>65</td><td>130</td><td>230</td><td>265</td><td>275</td><td>PRIMATE-120 <eq>[ABB^+14]</eq></td></tr><tr><td>(5,2,64)</td><td>320</td><td>18</td><td>35</td><td>65</td><td>130</td><td>258</td><td>300</td><td>315</td><td>ASCON Permutation <eq>[DEMS14]</eq></td></tr><tr><td rowspan="2">Target</td><td rowspan="2">Size(bits)</td><td colspan="7"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=9</td><td>r=10</td><td>r=11</td><td>r=12</td><td>r=13</td><td>r=14</td><td>r=15</td></tr><tr><td>(5,2,160)</td><td>800</td><td>258</td><td>515</td><td>705</td><td>770</td><td>790</td><td>798</td><td>-</td><td>KECCAK-f[800] <eq>[DBPA11]</eq></td></tr><tr><td>(5,2,256)</td><td>1280</td><td>258</td><td>515</td><td>1025</td><td>1195</td><td>1253</td><td>1271</td><td>1278</td><td></td></tr><tr><td>(5,2,320)</td><td>1600</td><td>258</td><td>515</td><td>1025</td><td>1410</td><td>1538</td><td>1580</td><td>1595</td><td>KECCAK-f[1600] <eq>[DBPA11]</eq></td></tr><tr><td rowspan="2">Target</td><td rowspan="2">Size(bits)</td><td colspan="7"><eq>\log_2(\#texts)</eq></td><td rowspan="2">Examples</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td></tr><tr><td>(5,4,40)</td><td>200</td><td>20</td><td>65</td><td>170</td><td>195</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(5,4,56)</td><td>280</td><td>20</td><td>65</td><td>230</td><td>270</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(5,4,64)</td><td>320</td><td>20</td><td>65</td><td>260</td><td>305</td><td>-</td><td>-</td><td>-</td><td></td></tr><tr><td>(5,4,160)</td><td>800</td><td>20</td><td>65</td><td>260</td><td>665</td><td>770</td><td>795</td><td>-</td><td></td></tr><tr><td>(5,4,256)</td><td>1280</td><td>20</td><td>65</td><td>260</td><td>1025</td><td>1220</td><td>1265</td><td>-</td><td>ICEPOLE Permutation <eq>[MGH^+14]</eq></td></tr><tr><td>(5,4,320)</td><td>1600</td><td>20</td><td>65</td><td>260</td><td>1025</td><td>1460</td><td>1565</td><td>1595</td><td></td></tr><tr><td rowspan="2">Target</td><td rowspan="2">Size(bits)</td><td colspan="7"><eq>\log_2(\#texts)</eq></td><td>Examples</td></tr><tr><td>r=3</td><td>r=4</td><td>r=5</td><td>r=6</td><td>r=7</td><td>r=8</td><td>r=9</td><td></td></tr><tr><td>(8,7,16)</td><td>128</td><td>56</td><td>120</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>AES <eq>[U.S01]</eq></td></tr><tr><td>(8,7,24)</td><td>192</td><td>56</td><td>176</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>Rijndael-192 <eq>[DR02]</eq></td></tr><tr><td>(8,7,32)</td><td>256</td><td>56</td><td>232</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>Rijndael-256 <eq>[DR02]</eq></td></tr><tr><td>(8,7,64)</td><td>512</td><td>56</td><td>344</td><td>488</td><td>-</td><td>-</td><td>-</td><td>-</td><td>WHIRLPOOL primitive <eq>[BR03]</eq></td></tr></table>