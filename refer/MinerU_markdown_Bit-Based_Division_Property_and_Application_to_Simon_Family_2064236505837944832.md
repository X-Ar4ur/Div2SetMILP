# Bit-Based Division Property and Application to Simon Family

Yosuke Todo1,2 and Masakatu Morii2 

1 NTT Secure Platform Laboratories, Tokyo, Japan 

todo.yosuke@lab.ntt.co.jp 

2 Kobe University, Kobe, Japan 

Abstract. Ciphers that do not use S-boxes have been discussed for the demand on lightweight cryptosystems, and their round functions consist of and, rotation, and xor. Especially, the Simon family is one of the most famous ciphers, and there are many cryptanalyses again the Simon family. However, it is very difficult to guarantee the security because we cannot use useful techniques for S-box-based ciphers. Very recently, the division property, which is a new technique to find integral characteristics, was shown in Eurocrypt 2015. The technique is powerful for S-box-based ciphers, and it was used to break, for the first time, the full MISTY1 in CRYPTO 2015. However, it has not been applied to non-S-box-based ciphers like the Simon family effectively, and only the existence of the 10-round integral characteristic on Simon32 was proven. On the other hand, the experimental characteristic, which possibly does not work for all keys, covers 15 rounds, and there is a 5-round gap. To fill the gap, we introduce a bit-based division property, and we apply it to show that the experimental 15-round integral characteristic always works for all keys. Though the bit-based division property finds more accurate integral characteristics, it requires much time and memory complexity. As a result, we cannot apply it to symmetric-key ciphers whose block length is over 32. Therefore, we alternatively propose a method for designers. The method works for ciphers with large block length, and it shows “provable security” against integral cryptanalyses using the division property. We apply this technique to the Simon family and show that Simon48, 64, 96, and 128 probably do not have 17-, 20-, 25-, and 29-round integral characteristics, respectively. 

Keywords: Integral cryptanalysis, Division property, Provable security, Simon family 

## 1 Introduction

Non-S-box-based ciphers have been proposed for the demand on lightweight cryptosystems [AJN15,BSS+13]. Such ciphers are superior in lightweight environments because they are implemented by logical operations and do not have a lookup table like S-boxes. In 2013, the NSA proposed a lightweight block cipher family, called the Simon family, that follows this design principle [BSS+13]. However, it is too difficult to guarantee the security against several cryptanalyses because we cannot use many useful techniques for S-boxbased ciphers. Therefore, many cryptanalyses have been proposed against the Simon family, e.g., [ALLW14,BRV14,BNS14,KLT15,SHW+14,WLV+14], and the designers recently summarized cryptanalyses in [BSS+15]. In this paper, we investigate the security of non-S-boxbased ciphers against integral cryptanalyses and illustrate our methods on the Simon family. 

Division Property Very recently, the division property, which is a new technique to find integral characteristics [KW02], was proposed in Eurocrypt 2015 [Tod15b]. The new technique permitted us to find a 6-round integral characteristic on MISTY1 in CRYPTO 2015, leading to the first complete theoretical cryptanalysis of the full MISTY1 [Tod15a]. Moreover, this technique was applied to generalized Feistel structures in [ZW15], leading to improved integral cryptanalyses against LBlock and TWINE. The division property also proves integral characteristics on the Simon family in [Tod15b], and Simon32, 48, 64, 96, and 128 have 9-, 11-, 11-, 13-, 13-round integral characteristics, respectively3. However, the round function is regarded as any function of degree 2. Therefore, we can expect that integral characteristics can be extended to more rounds if one is able to exploit the concrete structure of the round function. In fact, the experimental integral characteristic, which possibly does not work for all keys, covers 15 rounds $[ \mathrm { W L V ^ { + } 1 4 } ]$ , and there is a large gap between the proved characteristic and experimental one. 


Table 1. Integral characteristics on Simon32


<table><tr><td>Methods</td><td>#Rounds</td><td>Balanced bit (right half)</td><td>Reference</td></tr><tr><td>Experiment (no proof)</td><td>15</td><td>(?b??,????,b???,???b)</td><td>[WLV+14]</td></tr><tr><td>Division</td><td>10</td><td>(bbbb,bbbb,bbbb,bbbb)</td><td>[Tod15b]</td></tr><tr><td>Conventional bit-based division</td><td>14</td><td>(bbbb,bbbb,bbbb,bbbb)</td><td>Sect. 3</td></tr><tr><td>Bit-based division using 3 subsets</td><td>15</td><td>(?b??,????,b???,???b)</td><td>Sect. 4</td></tr></table>


Table 2. Provable secure number of rounds for the Simon family


<table><tr><td>Ciphers</td><td>SIMON48</td><td>SIMON64</td><td>SIMON96</td><td>SIMON128</td></tr><tr><td>Provable security</td><td>17 rounds</td><td>20 rounds</td><td>25 rounds</td><td>29 rounds</td></tr></table>

Our Contribution The round function of the Simon family is regarded as any function of degree 2 in [Tod15b] because we cannot decompose the round function into several sub blocks like S-boxes. However, we can decompose the round function into every bit, and we call the division property that focuses on every bit a bit-based division property. 

First, we apply the conventional bit-based division property to Simon32, which is not against the definition of the division property. Therefore, we can directly use the propagation rules of the division property. As a result, the conventional bit-based division property proves that Simon32 has a 14-round integral characteristic. However, there is still a gap of one round between the proof and experiment. Namely, this means that either the experimental 15-round characteristic does not work for all keys or the conventional bit-based division property cannot find the accurate characteristic. As a result, we conclude that the conventional bit-based division property is insufficient to find the accurate characteristic. The conventional division property divides the set of u according to whether the parity becomes 0 or unknown [Tod15b]. However, we should divide the set of u according to whether the parity becomes 0, 1, or unknown because we can also exploit the fact that the parity is not only 0 but also 1. To exploit this fact, we newly introduce a variant of the bitbased division property, which divides the set of u into three subsets. Since the variant is completely different from the definition of the conventional division property, we show the propagation characteristic also. Finally, we apply the variant to Simon32 and show that the experimental 15-round characteristic always works for all keys. The proved characteristic is the completely same as the experimental one including the position of balanced bits. Table 1 shows the comparison of integral characteristics, where balanced and unknown bits are labeled as b and ?, respectively. 

Although the bit-based division property can find more accurate integral characteristics, their propagations require much time and memory complexity. When we evaluate the propagation for n-bit block ciphers, it roughly requires 2n complexity because the bit-based division property has to manage the set of n-dimensional vectors whose elements take values in F2. This is feasible for Simon32 because the block length is 32 bits, but it is infeasible for other Simon family members. Therefore, we introduce a new technique, which is useful for designers but is not useful for attackers. We call this technique a lazy propagation, where we evaluate only a part of all propagations. The lazy propagation cannot find the integral characteristic, but it can evaluate the number of rounds that the bit-based division property cannot find integral characteristics even if we can evaluate the accurate propagation. Namely, the technique shows “provable security” for the integral cryptanalysis using the division property, and we expect that it becomes a useful technique for designers. Our provable security guarantees the security against only the integral cryptanalysis using the division property, and it does not always guarantee the security against all integral-like cryptanalyses. However, for Simon32, the bit-based division property can find the accurate integral characteristic. Therefore, we expect that it also finds the best integral characteristic for the other Simon family if it is feasible. Table 2 shows the number of rounds of Simon48, 64, 96, and 128, where the division property never finds integral characteristics. As a result, we expect that Simon48, 64, 96, and 128 do not have 17-, 20-, 25-, and 29-round integral characteristics, respectively4. 

## 2 Preliminaries

## 2.1 Notations

We make the distinction between the addition of Fn2 and addition of $\mathbb { Z } ,$ and we use ⊕ and + as the addition of Fn2 and addition of $\mathbb { Z } ,$ respectively. For any $a \in \mathbb { F } _ { 2 } ^ { n }$ , the ith element $a [ i ]$ $w ( a )$ $\begin{array} { r } { w ( a ) \ = \ \sum _ { i = 1 } ^ { n } a [ i ] } \end{array}$ For any $\pmb { a } \in ( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } \times \cdots \times \mathbb { F } _ { 2 } ^ { n _ { m } } )$ , the vectorial Hamming weight of a is defined as $W ( \pmb { a } ) = ( w ( a _ { 1 } ) , w ( a _ { 2 } ) , \dots , w ( a _ { m } ) ) \in \mathbb { Z } ^ { m }$ . Moreover, for any $\boldsymbol { k } \in \mathbb { Z } ^ { m }$ and $\pmb { k } ^ { \prime } \in \mathbb { Z } ^ { m }$ , we define $\pmb { k } \succeq k ^ { \prime } \mathrm { ~ i f ~ } k _ { i } \geq k _ { i } ^ { \prime }$ for all i. Otherwise, $\pm \kappa \neq k ^ { \prime }$ . In this paper, we often treat the set of k, and K denotes this set. Then, let |K| be the number of vectors. We simply write $\mathbb { K }  k$ when $\mathbb { K } : = \mathbb { K } \cup \{ k \}$ . Moreover, we simply write $\mathbb { K }  k$ , where the new K computed as 

$$
\mathbb{K} := \left\{ \begin{array}{l l} \mathbb{K} \cup \{\boldsymbol{k} \} & \text{if the original $\mathbb{K}$ does not include $\boldsymbol{k}$}, \\ \mathbb{K} \setminus \{\boldsymbol{k} \} & \text{if the original $\mathbb{K}$ includes $\boldsymbol{k}$}. \end{array} \right.
$$

## 2.2 Integral Attack

The integral attack was first introduced by Daemen et al. to evaluate the security of Square [DKR97], and then it was formalized by Knudsen and Wagner [KW02]. Attackers first prepare N chosen plaintexts and encrypt them R rounds. If the XOR of all encrypted texts becomes 0, we say that the cipher has an R-round integral characteristic with N chosen plaintexts. Finally, we analyze the entire cipher by using the integral characteristic. Therefore, it is very important to find integral characteristic. There are two main approaches to find integral characteristics. The first one is the propagation of the integral property [KW02] and the second one is based on the degree estimation [Knu94,Lai94]. 

## 2.3 Division Property

The division property, which was proposed in [Tod15b], is a new method to find integral characteristics. This section briefly shows the definition and propagation rules. Please refer to [Tod15b] in detail. 

Bit Product Function The division property of a multiset is evaluated by using the bit product function defined as follows. Let $\pi _ { u } : \mathbb { F } _ { 2 } ^ { n } \to \mathbb { F } _ { 2 }$ be a bit product function for any $u \in \mathbb { F } _ { 2 } ^ { n }$ . Let $\boldsymbol { x } \in \mathbb { F } _ { 2 } ^ { n }$ be the input, and $\pi _ { u } ( x )$ is the AND of $x [ i ]$ satisfying $u [ i ] = 1$ , i.e., it is defined as 

$$
\pi_ {u} (x) := \prod_ {i = 1} ^ {n} x [ i ] ^ {u [ i ]}.
$$

$x [ i ] ^ { 1 } = x [ i ]$ $x [ i ] ^ { 0 } = 1$ $\pi _ { \pmb { u } } : ( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m } } )  \mathbb { F } _ { 2 }$ $\pmb { u } \in ( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m } } )$ $\pmb { x } \in ( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m } } )$ and $\pi _ { \boldsymbol { u } } ( \boldsymbol { x } )$ is defined as 

$$
\pi_ {\boldsymbol {u}} (\boldsymbol {x}) := \prod_ {i = 1} ^ {m} \pi_ {u _ {i}} (x _ {i}).
$$

The bit product function also appears in the Algebraic Normal Form (ANF) of a Boolean function. The ANF of a Boolean function f is represented as 

$$
f (x) = \bigoplus_ {u \in \mathbb {F} _ {2} ^ {n}} a _ {u} ^ {f} \left(\prod_ {i = 1} ^ {n} x [ i ] ^ {u [ i ]}\right) = \bigoplus_ {u \in \mathbb {F} _ {2} ^ {n}} a _ {u} ^ {f} \pi_ {u} (x),
$$

where $\boldsymbol { a } _ { u } ^ { f } \in \mathbb { F } _ { 2 }$ is a constant value depending on f and u. 

## Definition of Division Property

Definition 1 (Division Property [Tod15b]). Let X be a multiset whose elements take a $( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m } } )$ . When the multiset X has the division property $\mathcal { D } _ { \mathbb { K } } ^ { n _ { 1 } , n _ { 2 } , \ldots , n _ { m } }$ , where K denotes a set of m-dimensional vectors whose ith element takes a value between 0 and $n _ { i }$ it fulfils the following conditions: 

$$
\bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {\boldsymbol {u}} (\boldsymbol {x}) = \left\{ \begin{array}{l l} u n k n o w n & i f t h e r e a r e \boldsymbol {k} \in \mathbb {K} s. t. W (\boldsymbol {u}) \succeq \boldsymbol {k}, \\ 0 & o t h e r w i s e. \end{array} \right.
$$

See [Tod15b] to better understand the concept in detail, and [SHZ+15] and [Tod15a] help readers understand the division property. In this paper, the division property for $( \mathbb { F } _ { 2 } ^ { n } ) ^ { m }$ is referred to as $\mathcal { D } _ { \mathbb { K } } ^ { n ^ { m } }$ for the simplicity5. If there are $k \in \mathbb { K }$ and $\pmb { k } ^ { \prime } \in \mathbb { K }$ satisfying $k \succeq k ^ { \prime }$ in the $\mathcal { D } _ { \mathbb { K } } ^ { n _ { 1 } , n _ { 2 } , \ldots , n _ { m } }$ 

Propagation Rules of Division Property Some propagation rules for the division property are proven in [Tod15b], and the rules are summarized in [Tod15a] as follows. 

Rule 1 (Substitution) Let F be a function that consists of m S-boxes, where the bit length and the algebraic degree of the ith S-box is $n _ { i }$ bits and $d _ { i } ,$ respectively. The $( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m } } )$ input multiset and output multiset, respectively. Assuming that the multiset X has the $\mathcal { D } _ { \mathbb { K } } ^ { n _ { 1 } , n _ { 2 } , \ldots , n _ { m } }$ $\mathbb { Y }$ $\mathcal { D } _ { \mathbb { K } ^ { \prime } } ^ { n _ { 1 } , n _ { 2 } , \ldots , n _ { m } }$ as 

$$
\mathbb {K} ^ {\prime} \leftarrow \left(\left\lceil \frac {k _ {1}}{d _ {1}} \right\rceil , \left\lceil \frac {k _ {2}}{d _ {2}} \right\rceil , \ldots , \left\lceil \frac {k _ {m}}{d _ {m}} \right\rceil\right), \forall \boldsymbol {k} \in \mathbb {K}.
$$

Here, when the ith S-box is bijective and $k _ { i } = n _ { i } .$ the ith element of the propagated property becomes $n _ { i }$ not $\lceil n _ { i } / d _ { i } \rceil$ . 

Rule 2 (Copy) Let F be a copy function, where the input x takes a value of $\mathbb { F } _ { 2 } ^ { n }$ and the output is calculated as $( y _ { 1 } , y _ { 2 } ) = ( x , x )$ . Let X and Y be the input multiset and output multiset, respectively. Assuming that the multiset X has the division property $\mathcal { D } _ { k } ^ { n }$ , the division property of the multiset Y is $\mathcal { D } _ { \mathbb { K } ^ { \prime } } ^ { n , n }$ as 

$$
\mathbb {K} ^ {\prime} \leftarrow (k - i, i), \text { for } 0 \leq i \leq k.
$$

Rule 3 (Compression by XOR) Let F be a function compressed by an XOR, where the input $( x _ { 1 } , x _ { 2 } )$ takes a value of $\left( \mathbb { F } _ { 2 } ^ { n } \times \mathbb { F } _ { 2 } ^ { n } \right)$ and the output is calculated as $y = x _ { 1 } \oplus x _ { 2 }$ . Let X and Y be the input multiset and output multiset, respectively. Assuming that the $\mathcal { D } _ { \mathbb { K } } ^ { n , n }$ $\mathbb { Y }$ $\mathcal { D } _ { k ^ { \prime } } ^ { n }$ as 

$$
k ^ {\prime} = \min _ {(k _ {1}, k _ {2}) \in \mathbb {K}} \{k _ {1} + k _ {2} \}.
$$

Here, if the minimum value of $k ^ { \prime }$ is larger than $n _ { : }$ , the propagation characteristic of the $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ $v \in \mathbb { F } _ { ? } ^ { n }$ 

Rule 4 (Split) Let F be a split function, where the input x takes a value of $\mathbb { F } _ { 2 } ^ { n }$ and the $x = y _ { 1 } \| y _ { 2 }$ $( y _ { 1 } , y _ { 2 } )$ $\left( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n - n _ { 1 } } \right)$ $\mathbb { Y }$ be the input multiset and output multiset, respectively. Assuming that the multiset X has the division property $\mathcal { D } _ { k } ^ { n }$ , the division property of the multiset $\mathbb { Y }$ is $\mathcal { D } _ { \mathbb { K } ^ { \prime } } ^ { n _ { 1 } , n - n _ { 1 } }$ as 

$$
\mathbb {K} ^ {\prime} \leftarrow (k - i, i), \text { for } 0 \leq i \leq k.
$$

Here, $( k - i )$ is less than or equal to $n _ { 1 }$ , and i is less than or equal to $n - n _ { 1 }$ . 

Rule 5 (Concatenation) Let $F$ be a concatenation function, where the input $( x _ { 1 } , x _ { 2 } )$ takes a value of $( \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \mathbb { F } _ { 2 } ^ { n _ { 2 } } )$ and the output is calculated as $y = x _ { 1 } \| x _ { 2 }$ . Let X and Y be the input multiset and output multiset, respectively. Assuming that the multiset X has $\mathcal { D } _ { \mathbb { K } } ^ { n _ { 1 } , n _ { 2 } }$ $\mathbb { Y }$ $\mathcal { D } _ { k ^ { \prime } } ^ { n _ { 1 } + n _ { 2 } }$ 

$$
k ^ {\prime} = \min _ {(k _ {1}, k _ {2}) \in \mathbb {K}} \{k _ {1} + k _ {2} \}.
$$

## 2.4 Simon Family

The Simon family is a lightweight block cipher family $\mathrm { [ B S S ^ { + } 1 3 ] }$ based on the Feistel construction. Let Simon2n be the Simon block ciphers with 2n-bit block length, where n is chosen from 16, 24, 32, 48, and 64. Moreover, Simon2n with mn-bit secret key is referred to as Simon2n/mn. Since we only care about integral characteristics on the Simon family, this paper only uses Simon2n. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/4f289e73-4343-47c0-b447-1a3f3cd1e7fd/9903b413b8c293bc259c0ccdbf487c50db0b899fc974127d146a210c407f0fa8.jpg)



Fig. 1. Round function of Simon2n


The output of the ith round function is denoted by $( L _ { i } , R _ { i } )$ and is calculated as 

$$
(L _ {i}, R _ {i}) = (L _ {i - 1} ^ {\lll 1} \wedge L _ {i - 1} ^ {\lll 8}) \oplus L _ {i - 1} ^ {\lll 2} \oplus R _ {i - 1} \oplus k _ {i}, L _ {i - 1}),
$$

where $L ^ { \ll j }$ denotes the j-bit left rotation of $L ,$ and $k _ { i }$ denotes the ith round key. Moreover, $( L _ { 0 } , R _ { 0 } )$ denotes a plaintext. The round function consists of and, rotation, and xor, and Fig. 1 shows the round function. For more details, please refer to [BSS+13]. 

## 2.5 Known Integral Characteristic on Simon Family

It is difficult to find effective integral characteristics on ciphers which consist of and, rotation, and xor. In $[ \mathrm { W L V ^ { + } 1 4 } ]$ , authors experimentally showed that Simon32 has the 15-round integral characteristic with $2 ^ { 3 1 }$ chosen plaintexts. Since their characteristic is confirmed under $2 ^ { 1 3 }$ secret keys, they expected that the success probability of this characteristic is at least $1 - 2 ^ { - 1 3 }$ . Therefore, this approach does not guarantee that the characteristic works for all secret keys. Moreover, it is practically infeasible to find integral characteristics of other Simon family members because the block length is too large for proceeding to an experimental evaluation. 

Integral characteristics proved under all secret keys are shown in [Tod15b], but in this approach the round function of Simon2n is seen as any n-bit function of degree 2. Therefore, the detailed structure of the round function is not exploited. As a result, it shows that Simon32, 48, 64, 96, and 128 has 9-, 11-, 11-, 13-, and 13-round integral characteristic, respectively. Since the round key is XORed after the round function, we can trivially get oneround extended integral characteristics using the same technique in $[ \mathrm { W L V ^ { + } 1 4 } ]$ . Therefore, 10-, 12-, 12-, 14-, and 14-round integral characteristics are proved in Simon32, 48, 64, 96, and 128, respectively. Thus, there is a 5-round gap between the proved characteristic and experimental one. 

## 3 Conventional Bit-Based Division Property

This paper introduces a bit-based division property. When n-bit block ciphers are analyzed, the conventional division property uses $\mathcal { D } _ { \mathbb { K } } ^ { \ell _ { 1 } , \hat { \ell } _ { 2 } , \dots , \ell _ { m } }$ , where $\ell _ { i }$ and m are chosen by attackers in $\begin{array} { r } { n = \sum _ { i = 1 } ^ { m } \ell _ { i } } \end{array}$ 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/4f289e73-4343-47c0-b447-1a3f3cd1e7fd/44e94c5407a88702ff77375495653ed2fee09eabf31b36ec9b9bc10e98ea5c63.jpg)



Fig. 2. Core operation of the Simon family.


i.e., $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { n } }$ . Since it is not against the definition of the conventional division property, we can directly use the five propagation rules shown in Sect. 2.3. 

## 3.1 Comparison between Conventional Bit-Based Division Property and Solving Algebraic Equations

Before the introduction of the conventional bit-based division property, we roughly show the relation between the bit-based division property and the resolution of algebraic equations by brute force. When entire ciphers are represented by algebraic equations, such equations involve both the plaintext and secret key. Therefore, if we solve such equations for an n-bit block cipher with a k-bit secret key, this roughly requires $2 ^ { k + n }$ complexity. On the other hand, XORing with a constant value does not change the conventional bit-based division property because such XORing is a linear function [Tod15a]. Therefore, the propagation of the conventional bit-based division property does not involve the secret key. It may miss some useful cryptographic properties, but it dramatically reduces the complexity. 

## 3.2 Propagation for Core Operation of Simon

As an example, we analyze Simon2n by using the conventional bit-based division property. We focus on only one bit of the right half in Simon2n. The core operation of the round function is represented by Fig. 2. Since the input and output bit length is 4 bits, we use the division property $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 4 } }$ . 

We consider the propagation characteristic. For instance, let assume that the input multiset has D14[k , $\mathcal { D } _ { [ k _ { 1 } , k _ { 2 } , k _ { 3 } , 1 ] } ^ { 1 ^ { 4 } } ,$ k ,k ,1], where ki denotes any value, i.e., 0 or 1. Then, if the multiset of $k _ { i }$ (y1, y2, y3, w5, x4) has D15[∗,∗, $( y _ { 1 } , y _ { 2 } , y _ { 3 } , w _ { 5 } , x _ { 4 } )$ $\bar { \mathcal { D } } _ { [ * , * , * , 1 , 1 ] } ^ { 1 ^ { 5 } }$ in the XOR, $x _ { 4 } \oplus w _ { 5 }$ . Consequently, the bit-based division property of $\left( { { y } _ { 1 } } , { { y } _ { 2 } } , { { y } _ { 3 } } , { { y } _ { 4 } } \right)$ is the same as that of $( x _ { 1 } , x _ { 2 } , x _ { 3 } , x _ { 4 } )$ . On the other hand, assuming that the input multiset has D 1 4 $\mathcal { D } _ { [ k _ { 1 } , k _ { 2 } , k _ { 3 } , 0 ] } ^ { 1 ^ { 4 } } .$ [k1,k2,k3,0], the output property is different from the input one. 

Let $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 4 } }$ and $\mathcal { D } _ { \mathbb { K } ^ { \prime } } ^ { 1 ^ { 4 } }$ be the division property of the input and output, respectively. When we get K0 from K, we first independently calculate vectors belonging to $\mathbb { K } ^ { \prime }$ by evaluating the propagation from every vector in K. Then, K0 is represented as the union of all calculated vectors. Finally, if there are $\pmb { k } \in \mathbb { K } ^ { \prime }$ and $\pmb { k } ^ { \prime } \in \mathbb { K } ^ { \prime }$ such that $k \succeq k ^ { \prime }$ , k is removed from K0 because the vector is redundant. 

$\mathcal { D } _ { k } ^ { 1 ^ { 4 } }$ $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 4 } }$ of Simon2n repeats the core operation for all n-bit values in the right half. Therefore, we $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 2 n } }$ independent of other $( 2 n - 4 )$ bits. Evaluating the propagation is a quite technical task. 


Table 3. Propagation of the conventional bit-based division property for the core operation in the Simon family


<table><tr><td>Input <eq>\mathcal{D}_{k}^{1^4}</eq></td><td>Output <eq>\mathcal{D}_{\mathbb{K}}^{1^4}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [0, 0, 0, 0]</eq></td><td><eq>\mathbb{K} = \{[0, 0, 0, 0]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [1, 0, 0, 0]</eq></td><td><eq>\mathbb{K} = \{[1, 0, 0, 0], [0, 0, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [0, 1, 0, 0]</eq></td><td><eq>\mathbb{K} = \{[0, 1, 0, 0], [0, 0, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [1, 1, 0, 0]</eq></td><td><eq>\mathbb{K} = \{[1, 1, 0, 0], [0, 0, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [0, 0, 1, 0]</eq></td><td><eq>\mathbb{K} = \{[0, 0, 1, 0], [0, 0, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [1, 0, 1, 0]</eq></td><td><eq>\mathbb{K} = \{[1, 0, 1, 0], [0, 0, 1, 1], [1, 0, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [0, 1, 1, 0]</eq></td><td><eq>\mathbb{K} = \{[0, 1, 1, 0], [0, 0, 1, 1], [0, 1, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [1, 1, 1, 0]</eq></td><td><eq>\mathbb{K} = \{[1, 1, 1, 0], [0, 0, 1, 1], [1, 1, 0, 1]\}</eq></td></tr><tr><td><eq>\boldsymbol{k} = [k_1, k_2, k_3, 1]</eq></td><td><eq>\mathbb{K} = \{[k_1, k_2, k_3, 1]\}</eq></td></tr></table>


Table 4. Size of K in $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 3 2 } }$ for the integral characteristic on Simon32


<table><tr><td>Round</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td></tr><tr><td><eq>|\mathbb{K}|</eq></td><td>1</td><td>1</td><td>3</td><td>11</td><td>65</td><td>774</td><td>18165</td><td>587692</td><td>5191387</td><td>1595164</td><td>95768</td><td>5894</td><td>682</td><td>136</td><td>32</td></tr></table>

Therefore, Appendix A shows a toy example to help readers understand the propagation. In the toy example, we evaluate the propagation on a Simon-like cipher whose block length is only 8 bits. 

## 3.3 Application to Simon32

We evaluate the propagation characteristic of the conventional bit-based division property on Simon32. We prepare chosen plaintexts such that the first bit is constant and the others $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 3 2 } }$ $\mathbb { K } =$ $\{ [ 0 , 1 , 1 , \ldots , 1 ] \}$ . Table 4 shows |K|, which is the number of vectors, in every round, where we perfectly remove redundant vectors from K. The output of the 14th round function has $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 3 2 } }$ Therefore, the conventional bit-based division property cannot show whether or not the output of the 14th round function is balanced. On the other hand, the output of the 13th $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 3 2 } }$ Hamming weight of the left half is 1 and that of the right half is 0, and $1 2 0 ~ ( = ~ { \binom { 1 6 } { 2 } } )$ vectors, whose Hamming weight of the left half is 0 and that of the right half is 2. This division property means that the output of the 13th round function takes the following integral property 

$$
(\ref {e q : 1})
$$

where balanced and unknown bits are labeled as b and ?, respectively. In the Simon family, since round keys are XORed with the right half only after the round function is applied to the left half, we can easily get a 14-round integral characteristic from the 13-round one. The same technique is used in $[ \mathrm { W L V ^ { + } } 1$ 4]. Therefore, we conclude that 14-round Simon32 has the integral characteristic with $2 ^ { 3 1 }$ chosen plaintexts. Appendix B shows the propagation and the integral characteristic in detail. 

## 4 Bit-Based Division Property using Three Subsets

## 4.1 Motivation

The conventional bit-based division property proved the existence of the 14-round integral characteristic of Simon32. However, the experimental characteristic covers 15 rounds $[ \mathrm { W L V ^ { + } 1 4 } ]$ , and there is still a one-round gap between the experiment and proof. In $[ \mathrm { W L V ^ { + } 1 4 } ]$ , the authors experimentally confirm the characteristic by randomly choosing $2 ^ { 1 3 }$ secret keys. Therefore, they concluded that the success probability of the characteristic is at least $1 - 2 ^ { - 1 3 }$ . Thus, we consider that this gap derives from either the experimental result does not work for all keys or the conventional bit-based division property cannot find the accurate characteristic. 

We first show that the conventional bit-based division property is insufficient to find integral characteristics on Simon32, and we then introduce a new variant of the bit-based division property. The conventional bit-based division property focuses on that the parity $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is 0 or unknown. On the other hand, the new variant focuses on that the parity $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is $0 , 1$ , or unknown. Therefore we call the new variant the bit-based division property using three subsets. The new variant can find more accurate integral characteristics and prove that the experimental characteristic shown in $[ \mathrm { W L V ^ { + } 1 4 } ]$ works for all keys. 

## 4.2 Characteristic that Conventional Bit-Based Division Property cannot Find

The conventional division property divides the set of u according to whether the parity becomes 0 or unknown [Tod15b]. However, it sometimes overlooks useful characteristics. We show it by using a simple example. 

We again evaluate the propagation of the conventional bit-based division property for the circuit in Fig 2, and $F : \mathbb { F } _ { 2 } ^ { 4 } \to \mathbb { F } _ { 2 } ^ { 4 }$ denotes the circuit. Moreover, let X and Y be the input $\mathcal { D } _ { \{ [ 1 , 1 , 0 , 0 ] , [ 0 , 0 , 1 , 0 ] \} } ^ { 1 ^ { 4 } } , \oplus _ { \pm \in \mathbb { X } } \pi _ { [ 1 , 1 , 0 , 0 ] } ( \pmb { x } )$ and $\begin{array} { r } { \bigoplus _ { { \pmb x } \in \mathbb { X } } \pi _ { [ 0 , 0 , 1 , 0 ] } ( { \pmb x } ) } \end{array}$ are unknown. Then, the output multiset Y has $\mathcal { D } _ { \{ [ 1 , 1 , 0 , 0 ] , [ 0 , 0 , 1 , 0 ] , [ 0 , 0 , 0 , 1 ] \} } ^ { 1 ^ { 4 } }$ from Table 3. 

Let us assume that both $\begin{array} { r } { \bigoplus _ { \pmb { x } \in \mathbb { X } } \pi _ { [ 1 , 1 , 0 , 0 ] } ( \pmb { x } ) } \end{array}$ and $\begin{array} { r } { \bigoplus _ { { \pmb x } \in \mathbb { X } } \pi _ { [ 0 , 0 , 1 , 0 ] } ( { \pmb x } ) } \end{array}$ are 1. Even if we know $\mathcal { D } _ { \{ [ 1 , 1 , 0 , 0 ] , [ 0 , 0 , 1 , 0 ] \} } ^ { 1 ^ { 4 } }$ . However, we can get the following equation. 

$$
\begin{array}{l} \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ 0, 0, 0, 1 ]} (F (\boldsymbol {x})) = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (x _ {1} x _ {2} \oplus x _ {3} \oplus x _ {4}) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (x _ {1} x _ {2}) \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (x _ {3}) \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (x _ {4}) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ 1, 1, 0, 0 ]} (\boldsymbol {x}) \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ 0, 0, 1, 0 ]} (\boldsymbol {x}) \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ 0, 0, 0, 1 ]} (\boldsymbol {x}) \\ = 1 \oplus 1 \oplus 0 = 0. \\ \end{array}
$$

Therefore, $\oplus _ { \pmb { x } \in \mathbb { X } } \pi _ { [ 0 , 0 , 0 , 1 ] } ( F ( \pmb { x } ) )$ is always 0 not unknown, and the division property of $\mathbb { Y }$ $\mathcal { D } _ { \{ [ 1 , 1 , 0 , 0 ] , [ 0 , 0 , 1 , 0 ] , [ 0 , 1 , 0 , 1 ] , [ 1 , 0 , 0 , 1 ] \} } ^ { 1 ^ { 4 } }$ $\mathcal { D } _ { \{ [ 1 , 1 , 0 , 0 ] , [ 0 , 0 , 1 , 0 ] , [ 0 , 0 , 0 , 1 ] \} } ^ { 1 ^ { 4 } } .$ 

Since the conventional division property focuses on the case the parity becomes 0, it cannot find characteristics that appear by cancelling like the above example. Therefore, we newly introduce a variant of the bit-based division property to exploit this fact. The variant divides the set of u into three subsets, i.e., 0, 1, and unknown. 

## 4.3 Definition of Bit-Based Division Property using Three Subsets

The conventional division property uses the set K to represent the subset of u such that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is unknown. The bit-based division property using three subsets needs to represent not only the subset of u such that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is unknown but also the subset of u such that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is one. Therefore, we use the set K to represent the subset of u such that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is unknown, and we also use the set L to represent the subset of u such that $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is one. 

Definition 2 (Bit-based division property using three subsets). Let X be a multiset whose elements take a value of $( \mathbb { F } _ { 2 } ) ^ { m }$ , and k is an m-dimensional vector whose ith element takes 0 or 1. When the multiset X has the bit-based division property using three subsets $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } }$ 

$$
\bigoplus_{\boldsymbol{x} \in \mathbb{X}} \pi_{\boldsymbol{u}} (\boldsymbol{x}) = \left\{ \begin{array}{l l} \text{unknown} & \text{if there are $\boldsymbol{k} \in \mathbb{K}$ s.t. $W(\boldsymbol{u}) \succeq \boldsymbol{k}$}, \\ 1 & \text{else if there is $\ell \in \mathbb{L}$ s.t. $W(\boldsymbol{u}) = \ell$}, \\ 0 & \text{otherwise}. \end{array} \right.
$$

If there are $k \in \mathbb { K }$ and $\pmb { k } ^ { \prime } \in \mathbb { K }$ satisfying $k \succeq k ^ { \prime }$ , k can be removed from K because the vector k is redundant. Moreover, when there is $k \in \mathbb { K }$ satisfying $W ( \boldsymbol { u } ) \ge k , \oplus _ { x \in \mathbb { X } } \pi _ { \boldsymbol { u } } ( x )$ is unknown even if there is $\ell \in \mathbb { L }$ satisfying $W ( \pmb { u } ) = \pmb { \ell }$ . Therefore, if there are $\ell \in \mathbb { L }$ and $k \in \mathbb { K }$ satisfying $\ell \succeq k .$ , the vector ` is redundant. Notice that redundant vectors in K and L do not affect whether $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ becomes 0, 1, or unknown for any u. 

Example 1. Let X be a multiset whose elements take a value of $( \mathbb { F } _ { 2 } ) ^ { 4 }$ . Assume the multiset ${ \mathcal { D } } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { 4 } }$ $\mathbb { K } = \{ [ 0 , 0 , 0 , 1 ] , [ 0 , 1 , 1 , 0 ] \}$ $\mathbb { L } =$ $\{ [ 1 , 0 , 0 , 0 ] , [ 1 , 0 , 1 , 0 ] , [ 0 , 0 , 1 , 0 ] , [ 0 , 0 , 1 , 1 ] \}$ . Then, every parity satisfies the following, where the value of u is represented as hexadecimal notation of $\left( u _ { 1 } \| u _ { 2 } \| u _ { 3 } \| u _ { 4 } \right)$ . 

<table><tr><td>u</td><td>0x0</td><td>0x1</td><td>0x2</td><td>0x3</td><td>0x4</td><td>0x5</td><td>0x6</td><td>0x7</td><td>0x8</td><td>0x9</td><td>0xA</td><td>0xB</td><td>0xC</td><td>0xD</td><td>0xE</td><td>0xF</td></tr><tr><td>Parity</td><td>0</td><td>?</td><td>1</td><td>?</td><td>0</td><td>?</td><td>?</td><td>?</td><td>1</td><td>?</td><td>1</td><td>?</td><td>0</td><td>?</td><td>?</td><td>?</td></tr></table>

Notice that the parity of $\pi _ { [ 0 , 0 , 1 , 1 ] } ( \pmb { x } )$ over all $\pmb { x } \in \mathbb { X }$ is unknown because there is $[ 0 , 0 , 0 , 1 ] \in \mathbb { K }$ and $W ( [ 0 , 0 , 1 , 1 ] ) \succeq W ( [ 0 , 0 , 0 , 1 ] )$ . Thus, $[ 0 , 0 , 1 , 1 ] \in \mathbb { I }$ is redundant. 

## 4.4 Propagation Rules

We show propagation rules for the bit-based division property using three subsets. There rules are very similar to those of the conventional division property. Here, we show three rules, “Copy,” “Compression by $\mathrm { A N D } , \mathfrak { r }$ and “Compression by $\mathrm { \ X O R } , \mathrm { \ Y }$ because any Boolean function can be evaluated by using these three rules. These rules are proved in Appendix C. 

Rule 1 (Copy) Let F be a copy function, where the input $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ takes values of $( \mathbb { F } _ { 2 } ) ^ { m }$ , and the output is calculated as $( x _ { 1 } , x _ { 1 } , x _ { 2 } , x _ { 3 } , \dots , x _ { m } )$ . Let X and Y be the input $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } }$ $\mathcal { D } _ { \mathbb { K } ^ { \prime } , \mathbb { L } ^ { \prime } } ^ { 1 ^ { m + 1 } }$ where $\mathbb { K } ^ { \prime }$ and $\mathbb { L } ^ { \prime }$ are computed as 

$$
\begin{array}{l} \mathbb {K} ^ {\prime} \leftarrow \left\{ \begin{array}{l l} (0, 0, k _ {2}, \ldots , k _ {m}), & \text { if } k _ {1} = 0 \\ (1, 0, k _ {2}, \ldots , k _ {m}), (0, 1, k _ {2}, \ldots , k _ {m}), & \text { if } k _ {1} = 1 \end{array} \right., \\ \mathbb {L} ^ {\prime} \leftarrow \left\{ \begin{array}{l l} (0, 0, \ell_ {2}, \ldots , \ell_ {m}), & \text { if } \ell_ {1} = 0 \\ (1, 0, \ell_ {2}, \ldots , \ell_ {m}), (0, 1, \ell_ {2}, \ldots , \ell_ {m}), (1, 1, \ell_ {2}, \ldots , \ell_ {m}) & \text { if } \ell_ {1} = 1 \end{array} \right.. \\ \end{array}
$$

from all $k \in \mathbb { K }$ and all $\ell \in \mathbb { L }$ , respectively. 

Rule 2 (Compression by AND) Let F be a function compressed by an AND, where the input $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ takes values of $( \mathbb { F } _ { 2 } ) ^ { m }$ , and the output is calculated as $( x _ { 1 } \wedge$ $x _ { 2 } , x _ { 3 } , \ldots , x _ { m } )$ . Let X and Y be the input multiset and output multiset, respectively. $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } } , \ \mathbb { Y }$ $\mathcal { D } _ { \mathbb { K } ^ { \prime } , \mathbb { L } ^ { \prime } } ^ { 1 ^ { m - 1 } }$ $\mathbb { K } ^ { \prime }$ $k \in \mathbb { K }$ 

$$
\mathbb {K} ^ {\prime} \leftarrow \left(\left\lceil \frac {k _ {1} + k _ {2}}{2} \right\rceil , k _ {3}, k _ {4}, \ldots , k _ {m}\right).
$$

Moreover, L0 is computed from all ` ∈ L s.t. (`1, `2) = (0, 0) or (1, 1) as 

$$
\mathbb {L} ^ {\prime} \leftarrow \left(\left\lceil \frac {\ell_ {1} + \ell_ {2}}{2} \right\rceil , \ell_ {3}, \ell_ {4}, \ldots , \ell_ {m}\right).
$$

Rule 3 (Compression by XOR) Let F be a function compressed by an XOR, where the input $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ takes values of $( \mathbb { F } _ { 2 } ) ^ { m }$ , and the output is calculated as $( x _ { 1 } \oplus$ $x _ { 2 } , x _ { 3 } , \ldots , x _ { m } )$ . Let X and Y be the input multiset and output multiset, respectively. $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } }$ $\mathcal { D } _ { \mathbb { K } ^ { \prime } , \mathbb { L } ^ { \prime } } ^ { 1 ^ { \bar { m } - 1 } }$ $\mathbb { K } ^ { \prime }$ $ { \boldsymbol { k } } \in \mathbb { K } \ \mathrm { s . t }$ $( k _ { 1 } , k _ { 2 } ) = ( 0 , 0 ) , ( 1 , 0 ) , \mathrm { o r ~ \bar { ( 0 , 1 ) } ~ }$ as 

$$
\mathbb {K} ^ {\prime} \leftarrow (k _ {1} + k _ {2}, k _ {3}, k _ {4}, \ldots , k _ {m}).
$$

Moreover, L0 is computed from all $\ell \in \mathbb { L } \mathrm { ~ s . t . ~ } ( \ell _ { 1 } , \ell _ { 2 } ) = ( 0 , 0 ) , ( 1 , 0 ) , \mathrm { ~ o r ~ } ( 0 , 1 )$ as 

$$
\mathbb {L} ^ {\prime} \stackrel {\mathbf {x}} {\leftarrow} \left(\ell_ {1} + \ell_ {2}, \ell_ {3}, \ell_ {4}, \dots , \ell_ {m}\right).
$$

## 4.5 Dependencies between K and L

Propagation for Public Function In the propagation rules shown in Sect. $4 . 4 , \mathbb { K } ^ { \prime }$ and $\mathbb { L } ^ { \prime }$ are computed from K and L, respectively. Therefore, we can evaluate the propagation from K and that from L independently. However, independent propagations generate many redundant vectors in $\mathbb { K } ^ { \prime }$ and L0. Note that redundant vectors in $\mathbb { K } ^ { \prime }$ and $\mathbb { L } ^ { \prime }$ do not affect whether the parity becomes 0, 1, or unknown for any u. Therefore, when we consider the propagation for public functions, we do not need to care about the dependencies between K and L. On the other hand, if there are many redundant vectors, the propagation requires much time complexity. Therefore, we should remove redundant vectors if possible because of the reason of only complexity. 

XORing with Secret Round Key For the public function, the propagation from K and that from L are independently evaluated. However, if the secret round key is XORed, every vector in L affects K. 

Let X and Y be the input and output multiset, respectively. Then, $\pmb { y } \in \mathbb { Y }$ is computed $\pmb { y } = \pmb { x } \oplus \pmb { r } \pmb { k }$ $\pmb { x } \in \mathbb { X }$ $^ { r k }$ $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } }$ L and D1mK0,L0 $\mathcal { D } _ { \mathbb { K } ^ { \prime } , \mathbb { L } ^ { \prime } } ^ { 1 ^ { m } }$ K',L' be the bit-based division property using three subsets on X and Y, respectively. We want to get K0 and L0 from K and L. We cannot know the secret round key. Therefore, the parity $\begin{array} { r } { \bigoplus _ { \pmb { x } \in \mathbb { X } } \pi _ { \pmb { v } } ( \pmb { x } \oplus \pmb { r } \pmb { k } ) } \end{array}$ satisfying $v \succ \ell$ becomes unknown because the parity depends on the secret round key. 

In many ciphers, round keys are XORed with a part of entire bits. Assuming a round key is XORed with the ith bit, K0 is computed as 

$$
\mathbb {K} ^ {\prime} \leftarrow (\ell_ {1}, \ell_ {2}, \dots , \ell_ {i} \vee 1, \dots , \ell_ {m})
$$

for all $\ell \in \mathbb { L }$ satisfying $\ell _ { i } = 0$ . 


Table 5. Propagation of the bit-based division property using three subsets for the core operation in the Simon family


<table><tr><td>Input <eq>\mathcal{D}_{\mathbb{K},\{\ell\}}^{1^4}</eq></td><td>Output <eq>\mathcal{D}_{\mathbb{K}&#x27;,\mathbb{L}&#x27;}^{1^4}</eq></td></tr><tr><td><eq>\ell = [0,0,0,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[0,0,0,0]\}</eq></td></tr><tr><td><eq>\ell = [1,0,0,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[1,0,0,0]\}</eq></td></tr><tr><td><eq>\ell = [0,1,0,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[0,1,0,0]\}</eq></td></tr><tr><td><eq>\ell = [1,1,0,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[1,1,0,0], [0,0,0,1], [1,0,0,1], [0,1,0,1], [1,1,0,1]\}</eq></td></tr><tr><td><eq>\ell = [0,0,1,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[0,0,1,0], [0,0,0,1], [0,0,1,1]\}</eq></td></tr><tr><td><eq>\ell = [1,0,1,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[1,0,1,0], [1,0,0,1], [1,0,1,1]\}</eq></td></tr><tr><td><eq>\ell = [0,1,1,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[0,1,1,0], [0,1,0,1], [0,1,1,1]\}</eq></td></tr><tr><td><eq>\ell = [1,1,1,0]</eq></td><td><eq>\mathbb{L}&#x27; = \{[1,1,1,0], [0,0,1,1], [1,0,1,1], [0,1,1,1], [1,1,0,1]\}</eq></td></tr><tr><td><eq>\ell = [\ell_1,\ell_2,\ell_3,1]</eq></td><td><eq>\mathbb{L}&#x27; = \{[\ell_1,\ell_2,\ell_3,1]\}</eq></td></tr></table>


$\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { 3 2 } }$ for the integral characteristic on Simon32


<table><tr><td>Round</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td><eq>|\mathbb{L}|</eq></td><td>1</td><td>1</td><td>5</td><td>19</td><td>138</td><td>2236</td><td>89878</td><td>4485379</td><td>47149981</td><td>2453101</td><td>20360</td><td>168</td><td>8</td><td>0</td><td>0</td><td>0</td></tr><tr><td><eq>|\mathbb{K}|</eq></td><td>1</td><td>1</td><td>1</td><td>6</td><td>43</td><td>722</td><td>23321</td><td>996837</td><td>9849735</td><td>2524718</td><td>130724</td><td>7483</td><td>852</td><td>181</td><td>32</td><td>32</td></tr></table>

## 4.6 Propagation for Core Operation of Simon

We search for integral characteristics on Simon32 by the bit-based division property using three subsets. Similar to the conventional bit-based division property, we focus on only one bit of the right half and consider the core operation of the Simon family (see Fig. 2). 

The core operation is a public function and it does not involve any secret information. Therefore, we can evaluate the propagation from K and that from L independently. Table 5 ${ \mathcal { D } } _ { \mathbb { K } , \{ \ell \} } ^ { 1 ^ { 4 } }$ $\mathcal { D } _ { \mathbb { K } ^ { \prime } , \mathbb { L } ^ { \prime } } ^ { 1 ^ { 4 } }$ D K0,L0 , where the propagation from K to $\mathbb { K } ^ { \prime }$ is the same as that in Table 3. Appendix D helps readers understand the propagation. Next, the propagation on the round function can be evaluated by repeating for all bits of the right half. Finally, when round keys are XORed with the right half, new vectors are generated from $\mathbb { L } ,$ and the new vectors are inserted into K. Appendix E shows a toy example, which is the same as Appendix A, to help readers understand the propagation. 

## 4.7 Application to Simon32

We evaluate the propagation characteristic of the bit-based division property using three subsets on Simon32. We prepare chosen plaintexts such that the first bit is constant and $\mathcal { D } _ { \{ [ 1 , 1 , 1 , . . . , 1 ] \} , \{ [ 0 , 1 , 1 , . . . , 1 ] \} } ^ { 1 ^ { 3 2 } } .$ 

Table 6 shows $| \mathbb { K } |$ and |L| in every round, where we perfectly remove redundant vectors ${ \mathcal { D } } _ { \mathbb { K } , \phi } ^ { 1 ^ { 3 2 } }$ , where vector in K are represented by hexadecimal notation as 

```txt
(0001 0000)(0002 0000)(0004 0000)(0008 0000)(0010 0000)(0020 0000)(0040 0000)(0080 0000)
(0100 0000)(0200 0000)(0400 0000)(0800 0000)(1000 0000)(2000 0000)(4000 0000)(8000 0000)
(0000 0002)(0000 0004)(0000 0008)(0000 0010)(0000 0020)(0000 0040)(0000 0081)(0000 0100)
(0000 0200)(0000 0400)(0000 0800)(0000 1000)(0000 2000)(0000 4001)(0000 488) (OOOO 8OOO), 
```

and $\phi$ denotes the empty set. This division property means that the output of the 14th round function takes the following integral property 

```csv
(????,????,????,????,?b??,????,b???,???b), 
```

where balanced and unknown bits are labeled as b and ?, respectively. In the Simon family, we can easily get a 15-round integral characteristic from the 14-round one, and this proved integral characteristic is completely the same as the experimental one. Therefore, we conclude that the experimental characteristic is not probabilistic characteristic, and it works for all keys. Appendix F shows the propagation and the integral characteristic in detail. 

## 4.8 Application to Simeck32

Simeck was recently proposed in $[ \mathrm { Y } \mathrm { Z } \mathrm { S } ^ { + } 1 5 ]$ , and its round function is very similar to that of Simon. Let $( L _ { i } , R _ { i } )$ be the output of the ith round function, and it is calculated as 

$$
(L _ {i}, R _ {i}) = (L _ {i - 1} \wedge L _ {i - 1} ^ {\lll 5}) \oplus L _ {i - 1} ^ {\lll 1} \oplus R _ {i - 1} \oplus k _ {i}, L _ {i - 1}).
$$

The rotation number is changed from (1, 8, 2) to $^ { ( 0 , 5 , 1 ) }$ . Similar to Simon, Simeck has different parameters according to the block length. Let Simeck2n be the Simeck block ciphers with 2n-bit block length, where n is chosen from 16, 24, and 32. 

We also evaluated the propagation of the bit-based division property using three subsets ${ \mathcal { D } } _ { \mathbb { K } , \phi } ^ { 1 ^ { 3 2 } }$ in K are represented by hexadecimal notation as 

```txt
(0001 0000) (0002 0000) (0004 0000) (0008 0000) (0010 0000) (0020 0000) (0040 0000) (0080 0000)
(0100 0000) (0200 0000) (0400 0000) (0800 0000) (1000 0000) (2000 0000) (4000 0000) (8000 0000)
(0000 0002) (0000 0004) (0000 0008) (0000 0011) (0000 0021) (0000 0030) (0000 0040) (0000 0080)
(0000 0100) (0000 0201) (0000 0210) (0000 0220) (0000 0401) (0000 0410) (0000 0420) (0000 0600)
(0000 0800) (0000 1000) (0000 2000) (0000 4001) (0000 411) (1999) (422) (1999) (422) (1999) (422) (1999) (422) (1999) (422) (1999) (422) (1999) (422) (1999) (422) (1999) (422) (1999) (422). 
```

This division property means that the output of the 14th round function takes the following integral property 

$$
(\ref {e q : 1})
$$

Since round keys are XORed after the round function in Simeck, we can trivially get the 15-round integral characteristic. Here, $2 ^ { 3 1 }$ plaintexts are chosen as $( L _ { 0 } , F ( L _ { 0 } ) \oplus R _ { 0 } )$ , where the first bit of $R _ { 0 }$ is constant and the others are active. 

## 5 Provable Security against Integral Cryptanalysis

We introduced the bit-based division property using three subsets in Sect. 4, and we proved that this method can find more accurate integral characteristics than those found by the conventional division property. In particular, we showed that the new method can discover the tight characteristic on Simon32. However, a problem is left about the feasibility, $\mathrm { i . e . }$ , the propagation of the division property requires much time and memory complexity. For instance, if we want to evaluate the propagation of the division property $\mathcal { D } _ { \mathbb { K } } ^ { n ^ { m } }$ , the time and memory complexity is upper-bounded by $( n { + } 1 ) ^ { m }$ . Therefore, if the upper bound is too large, $\mathrm { e . g . , ~ } ( n + 1 ) ^ { m } \gg 2 ^ { 3 2 }$ , it is difficult to evaluate the propagation 6. In the bit-based division property, the time and memory complexity is upper-bounded by $2 ^ { n }$ , where n denotes the block length. Moreover, the bit-based division property using three subsets requires more complexity than that using two subsets. Therefore, we cannot apply the bit-based division property to the Simon family except for Simon32. 

## 5.1 Provable Security for Designers

We cannot apply the bit-based division property to the Simon family except for Simon32, but we can show the “provable security” alternatively. When we design new symmetrickey primitives, we have to guarantee the security against several cryptanalyses. Provable security has been discussed in detail for differential and linear cryptanalyses [Mat96,NK95], but such tools do not exist for integral cryptanalysis. 

Let D1mKi,L $\mathcal { D } _ { \mathbb { K } _ { i } , \mathbb { L } _ { i } } ^ { 1 ^ { m } }$ denotes the division property of the output set of the ith round function. We want to find R-round integral characteristics. Then, for any u with $w ( \pmb { u } ) = 1$ , we have to evaluate that there are not $\boldsymbol { k } \in \mathbb { K } _ { R }$ satisfying $W ( { \pmb u } ) \succeq { \pmb k }$ and $\ell \in \mathbb { L } _ { R }$ satisfying $W ( \pmb { u } ) = \pmb { \ell } .$ . Therefore, we have to get all vectors in $\mathbb { K } _ { R }$ and $\mathbb { L } _ { R }$ , and such vectors are searched by an algorithm like breadth-first search. On the other hand, we want to show that an R-round integral characteristic cannot exist. Then, it is enough to show that $\mathbb { K } _ { R }$ has m distinct vectors whose Hamming weight is one, i.e., there is not balanced bits, and such vectors are searched by an algorithm like depth-first search. In our provable security, we aim to get such number of rounds efficiently, and a lazy propagation is useful to find such number of rounds. 

$\mathcal { D } _ { \mathbb { K } _ { i - 1 } , \mathbb { L } _ { i - 1 } } ^ { 1 ^ { m } }$ be the bit-based division property of $\mathcal { D } _ { \mathbb { \bar { K } } _ { i } , \bar { \mathbb { L } } _ { i } } ^ { 1 ^ { m } }$ be the bit-based division property from the lazy propagation. Then, $\bar { { \mathbb K } } _ { i }$ is computed from only a part of vectors in $\mathbb { K } _ { i - 1 }$ , and $\mathbb { L } _ { i }$ always becomes the empty set $\phi$ . 

The lazy propagation first removes all vectors from $\mathbb { L } _ { i - 1 }$ . Moreover, it only evaluates the propagation from vectors with low Hamming weight in $\mathbb { K } _ { i - 1 }$ because such vectors are more close to unknown. Therefore, it is more efficiently evaluated than the accurate propagation. 

Let us consider the meaning of the lazy propagation. Assuming the input set of the $( i - 1 )$ $\mathcal { D } _ { \mathbb { K } _ { i - 1 } , \mathbb { L } _ { i - 1 } } ^ { 1 ^ { m } }$ $\mathcal { D } _ { \mathbb { K } _ { i } , \mathbb { L } _ { i } } ^ { 1 ^ { m } }$ and D1m¯ $\mathcal { D } _ { \mathbb { R } _ { i } , \phi } ^ { 1 ^ { m } }$ by the accurate propagation and the lazy propagation, respectively. Then, the set of u that the parity is unknown is represented as 

$$
\mathbb {S} _ {\mathbb {K}} := \left\{\boldsymbol {u} \in (\mathbb {F} _ {2}) ^ {m} \mid \text { there   are } \boldsymbol {k} \in \mathbb {K} _ {i} \text { satisfying } W (\boldsymbol {u}) \succeq \boldsymbol {k} \right\}.
$$

On the other hand, $\mathbb { S } _ { \bar { \mathbb { K } } _ { i } }$ cannot completely represent the set of u that the parity is unknown. However, $\mathbb { S } _ { \bar { \mathbb { K } } _ { i } } \subseteq \mathbb { S } _ { \mathbb { K } _ { i } }$ i always holds. 

$\mathcal { D } _ { \mathbb { K } _ { i } , \phi } ^ { 1 ^ { m } }$ D $\mathcal { D } _ { \mathbb { K } _ { i + 1 } , \mathbb { L } _ { i + 1 } } ^ { 1 ^ { m } }$ 1m $\mathcal { D } _ { \mathbb { K } _ { i + 1 } , \phi } ^ { 1 ^ { m } }$ is propagated fromhe division property $\mathcal { D } _ { \mathbb { K } _ { i } , \mathbb { L } _ { i } } ^ { 1 ^ { m } }$ $\mathbb { S } _ { \mathbb { K } _ { i + 1 } } \subseteq \mathbb { S } _ { \mathbb { K } _ { i + 1 } }$ always holds because $\mathbb { S } _ { \bar { \mathbb { K } } _ { i } } \ \subseteq \mathbb { S } _ { \mathbb { K } _ { i } }$ . ${ \mathcal { D } } _ { \mathbb { \bar { K } } _ { R } , \phi } ^ { 1 ^ { m } } .$ $\bar { \mathbb { K } } _ { R }$ Hamming weight is one, the accurate propagation also creates the same m distinct vectors in the same round. 

## 5.2 Application to Simon Family

We evaluate the lazy propagation of the bit-based division property on Simon48, Simon64, Simon96, and Simon128. Here, we only evaluate integral characteristics when they use chosen plaintexts that only one bit of the left half is constant and the other bits are active. We calculate the accurate propagation up to 6 rounds7 Table 7 shows $\mathrm { m i n } _ { w } ( \mathbb { L } )$ and $\mathrm { m i n } _ { w } ( \mathbb { K } )$ 


Table 7. Accurate propagations up to six rounds


<table><tr><td rowspan="2">#rounds</td><td colspan="2">SIMON48</td><td colspan="2">SIMON64</td><td colspan="2">SIMON96</td><td colspan="2">SIMON128</td></tr><tr><td><eq>\min_w(\mathbb{L})</eq></td><td><eq>\min_w(\mathbb{K})</eq></td><td><eq>\min_w(\mathbb{L})</eq></td><td><eq>\min_w(\mathbb{K})</eq></td><td><eq>\min_w(\mathbb{L})</eq></td><td><eq>\min_w(\mathbb{K})</eq></td><td><eq>\min_w(\mathbb{L})</eq></td><td><eq>\min_w(\mathbb{K})</eq></td></tr><tr><td>0</td><td>47</td><td>48</td><td>63</td><td>64</td><td>95</td><td>96</td><td>127</td><td>128</td></tr><tr><td>1</td><td>47</td><td>48</td><td>63</td><td>64</td><td>95</td><td>96</td><td>127</td><td>128</td></tr><tr><td>2</td><td>46</td><td>47</td><td>62</td><td>63</td><td>94</td><td>96</td><td>126</td><td>128</td></tr><tr><td>3</td><td>45</td><td>46</td><td>61</td><td>62</td><td>93</td><td>94</td><td>125</td><td>126</td></tr><tr><td>4</td><td>43</td><td>44</td><td>59</td><td>60</td><td>91</td><td>92</td><td>123</td><td>124</td></tr><tr><td>5</td><td>40</td><td>41</td><td>56</td><td>57</td><td>88</td><td>89</td><td>120</td><td>121</td></tr><tr><td>6</td><td>35</td><td>36</td><td>51</td><td>52</td><td>83</td><td>84</td><td>115</td><td>116</td></tr></table>


Table 8. Lazy propagation of the bit-based division property for the Simon family


<table><tr><td rowspan="2">#rounds</td><td colspan="2">SIMON48</td><td colspan="2">SIMON64</td><td colspan="2">SIMON96</td><td colspan="2">SIMON128</td></tr><tr><td><eq>\min_w(\mathbb{K})</eq></td><td>Limit</td><td><eq>\min_w(\mathbb{K})</eq></td><td>Limit</td><td><eq>\min_w(\mathbb{K})</eq></td><td>Limit</td><td><eq>\min_w(\mathbb{K})</eq></td><td>Limit</td></tr><tr><td>7</td><td>30</td><td>33</td><td>46</td><td>61</td><td>78</td><td>81</td><td>110</td><td>113</td></tr><tr><td>8</td><td>20</td><td>23</td><td>35</td><td>38</td><td>68</td><td>71</td><td>100</td><td>103</td></tr><tr><td>9</td><td>11</td><td>14</td><td>23</td><td>26</td><td>55</td><td>57</td><td>87</td><td>88</td></tr><tr><td>10</td><td>7</td><td>10</td><td>13</td><td>15</td><td>40</td><td>41</td><td>71</td><td>71</td></tr><tr><td>11</td><td>5</td><td>8</td><td>9</td><td>10</td><td>27</td><td>28</td><td>59</td><td>59</td></tr><tr><td>12</td><td>3</td><td>8</td><td>6</td><td>8</td><td>17</td><td>17</td><td>42</td><td>42</td></tr><tr><td>13</td><td>2</td><td>5</td><td>4</td><td>7</td><td>11</td><td>11</td><td>32</td><td>32</td></tr><tr><td>14</td><td>2</td><td>3</td><td>3</td><td>7</td><td>8</td><td>9</td><td>21</td><td>21</td></tr><tr><td>15</td><td>1</td><td>2</td><td>2</td><td>7</td><td>5</td><td>6</td><td>15</td><td>15</td></tr><tr><td>16</td><td>1(u)</td><td>1</td><td>2</td><td>4</td><td>4</td><td>6</td><td>10</td><td>10</td></tr><tr><td>17</td><td></td><td></td><td>1</td><td>3</td><td>3</td><td>6</td><td>8</td><td>8</td></tr><tr><td>18</td><td></td><td></td><td>1</td><td>1</td><td>2</td><td>6</td><td>5</td><td>6</td></tr><tr><td>19</td><td></td><td></td><td>1(u)</td><td>1</td><td>2</td><td>6</td><td>4</td><td>6</td></tr><tr><td>20</td><td></td><td></td><td></td><td></td><td>1</td><td>6</td><td>3</td><td>6</td></tr><tr><td>21</td><td></td><td></td><td></td><td></td><td>1</td><td>6</td><td>2</td><td>6</td></tr><tr><td>22</td><td></td><td></td><td></td><td></td><td>1</td><td>6</td><td>2</td><td>6</td></tr><tr><td>23</td><td></td><td></td><td></td><td></td><td>1</td><td>1</td><td>2</td><td>6</td></tr><tr><td>24</td><td></td><td></td><td></td><td></td><td>1(u)</td><td>1</td><td>1</td><td>6</td></tr><tr><td>25</td><td></td><td></td><td></td><td></td><td></td><td></td><td>1</td><td>6</td></tr><tr><td>26</td><td></td><td></td><td></td><td></td><td></td><td></td><td>1</td><td>6</td></tr><tr><td>27</td><td></td><td></td><td></td><td></td><td></td><td></td><td>1</td><td>6</td></tr><tr><td>28</td><td></td><td></td><td></td><td></td><td></td><td></td><td>1(u)</td><td>1</td></tr></table>

$\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { 2 n } }$ $\mathrm { m i n } _ { w } ( \mathbb { L } )$ $\mathrm { m i n } _ { w } ( \mathbb { K } )$ calculated as 

$$
\min _ {w} (\mathbb {K}) = \min _ {\boldsymbol {k} \in \mathbb {K}} \left(\sum_ {i = 1} ^ {2 n} w (k _ {i})\right), \quad \min _ {w} (\mathbb {L}) = \max _ {\boldsymbol {\ell} \in \mathbb {L}} \left(\sum_ {i = 1} ^ {2 n} w (\ell_ {i})\right).
$$

From the 7th round function, we repeat the lazy propagation. We first remove all vectors ${ \mathcal { D } } _ { \mathbb { K } , \phi } ^ { 1 ^ { 2 n } } .$ $\phi$ the empty set. Moreover, we remove vectors with high Hamming weight from K. Table 8 $\bar { \mathcal { D } } _ { \mathbb { K } , \phi } ^ { 1 ^ { 2 n } }$ vectors $k \in \mathbb { K }$ satisfying 

$$
\min _ {w} (\mathbb {K}) \leq \sum_ {i = 1} ^ {2 n} w (k _ {i}) \leq \text { Limit }.
$$

Here, u means that the K has 2n distinct vectors whose Hamming weight is one, and then, we simply say that the propagation reaches the unknown. 

$k \in \mathbb { K }$ $< \textstyle \sum _ { i = 1 } ^ { 2 n } w ( k _ { i } )$ , we do not evaluate the propagation from the k. Therefore, if the propagation from the removed vector k immediately reaches the unknown, there is a gap between the accurate propagation and the lazy propagation. However, if the lazy propagation reaches the unknown in a specific number of rounds, the accurate propagation at least reaches the unknown in the same number of rounds. Therefore, the lazy propagation is not useful for attackers, but it guarantees the number of rounds that the bit-based division property cannot find integral characteristics. 

As a result, the lazy propagation shows that 16-, 19-, 24-, and 28-round Simon48, 64, 96, and 128 probably do not have integral characteristics, respectively. However, we can get $( r + 1 )$ )-round integral characteristics from r-round integral characteristics because round keys are XORed after the round function. Therefore, we expect that 17-, 20-, 25-, and 29- round Simon48, 64, 96, and 128 probably do not have integral characteristics, respectively. 

## 5.3 Characteristics that Bit-Based Division Property cannot Find

We consider characteristics that the bit-based division property cannot find. Our provable security supposes that all round keys are randomly and secretly chosen. However, practical ciphers generate round keys from the secret key using the key scheduling algorithm. Therefore, our provable security does not suppose integral characteristics that exploit the key scheduling algorithm. 

The bit-based division property using three subsets focuses on the parity $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ , and divide the set of u into three subsets. Then, the propagation simply regard $\begin{array} { r } { \bigoplus _ { \pmb { x } \in \mathbb { X } } \pi _ { \pmb { u } _ { 1 } } ( \pmb { x } ) \bigoplus } \end{array}$ $\pi _ { \boldsymbol { u } _ { 2 } } ( \boldsymbol { x } )$ as unknown if either $\oplus _ { { \pmb x } \in \mathbb { X } } \pi _ { { \pmb u } _ { 1 } } ( { \pmb x } )$ or $\oplus _ { x \in \mathbb { X } } \pi _ { { \pmb u } _ { 2 } } ( { \pmb x } )$ is unknown. For instance, if $\oplus _ { { \pmb x } \in \mathbb { X } } \pi _ { { \pmb u } _ { 1 } } ( { \pmb x } ) \oplus \pi _ { { \pmb u } _ { 2 } } ( { \pmb x } )$ is always 0 or 1 although $\oplus _ { { \pmb x } \in \mathbb { X } } \pi _ { { \pmb u } _ { 1 } } ( { \pmb x } )$ and $\oplus _ { x \in \mathbb { X } } \pi _ { { \mathbf { * } } _ { 2 } } ( { \mathbf { * } } )$ are unknown, the bit-based division property cannot exploit such property. 

## 6 Conclusions

The division property is a useful technique to find integral characteristics, but it has not been applied to non-S-box-based ciphers effectively. This paper focused on the bit-based division property. More precisely, this paper proposed a new variant using three subsets. The conventional bit-based division property divides the set of u into two subsets, but the new variant divides the set of u into three subsets. The bit-based division property using three subsets can prove that the experimental integral characteristic for Simon32 shown in $[ \mathrm { W L V ^ { + } 1 4 } ]$ works for all keys. Moreover, we focused on the propagation of the division property. Then, we showed that the lazy propagation is useful to guarantee the security against integral cryptanalyses using the division property. As a result, we showed that 17-, 20-, 25-, and 29-round Simon48, 64, 96, and 128 probably do not have integral characteristics, respectively. 

Acknowledgments. The authors would like to thank the anonymous referees for their helpful comments. 

## References



AJN15. Jean-Philippe Aumasson, Philipp Jovanovic, and Samuel Neves. Norx v2.0, 2015. Submission to CAESAR competition. 





ALLW14. Farzaneh Abed, Eik List, Stefan Lucks, and Jakob Wenzel. Differential cryptanalysis of roundreduced simon and speck. In Carlos Cid and Christian Rechberger, editors, FSE 2014, volume 8540 of LNCS, pages 525–545. Springer, 2014. 





BNS14. Christina Boura, Mar´ıa Naya-Plasencia, and Valentin Suder. Scrutinizing and improving impossible differential attacks: Applications to CLEFIA, Camellia, LBlock and Simon. In Palash Sarkar and Tetsu Iwata, editors, ASIACRYPT 2014 Part I, volume 8873 of LNCS, pages 179–199. Springer, 2014. 





BRV14. Alex Biryukov, Arnab Roy, and Vesselin Velichkov. Differential analysis of block ciphers SIMON and SPECK. In Carlos Cid and Christian Rechberger, editors, FSE 2014, volume 8540 of LNCS, pages 546–570. Springer, 2014. 





BSS+13. Ray Beaulieu, Douglas Shors, Jason Smith, Stefan Treatman-Clark, Bryan Weeks, and Louis Wingers. The SIMON and SPECK families of lightweight block ciphers. IACR Cryptology ePrint Archive, 2013:404, 2013. 





BSS+15. Ray Beaulieu, Douglas Shors, Jason Smith, Stefan Treatman-Clark, Bryan Weeks, and Louis Wingers. SIMON and SPECK: block ciphers for the internet of things. IACR Cryptology ePrint Archive, 2015:585, 2015. 





DKR97. Joan Daemen, Lars R. Knudsen, and Vincent Rijmen. The block cipher Square. In Eli Biham, editor, FSE, volume 1267 of LNCS, pages 149–165. Springer, 1997. 





KLT15. Stefan K¨olbl, Gregor Leander, and Tyge Tiessen. Observations on the SIMON block cipher family. In Rosario Gennaro and Matthew Robshaw, editors, CRYPTO 2015 Part I, volume 9215, pages 161–185. Springer, 2015. 





Knu94. Lars R. Knudsen. Truncated and higher order differentials. In Bart Preneel, editor, FSE, volume 1008 of LNCS, pages 196–211. Springer, 1994. 





KW02. Lars R. Knudsen and David Wagner. Integral cryptanalysis. In Joan Daemen and Vincent Rijmen, editors, FSE, volume 2365 of LNCS, pages 112–127. Springer, 2002. 





Lai94. Xuejia Lai. Higher order derivatives and differential cryptanalysis. In Communications and Cryptography, volume 276 of The Springer International Series in Engineering and Computer Science, pages 227–233, 1994. 





Mat96. Mitsuru Matsui. New structure of block ciphers with provable security against differential and linear cryptanalysis. In Dieter Gollmann, editor, FSE, volume 1039 of LNCS, pages 205–218. Springer, 1996. 





NK95. Kaisa Nyberg and Lars R. Knudsen. Provable security against a differential attack. J. Cryptology, 8(1):27–37, 1995. 





SHW+14. Siwei Sun, Lei Hu, Peng Wang, Kexin Qiao, Xiaoshuang Ma, and Ling Song. Automatic security evaluation and (related-key) differential characteristic search: Application to simon, present, lblock, DES(L) and other bit-oriented block ciphers. In Palash Sarkar and Tetsu Iwata, editors, ASIACRYPT 2014 Part I, volume 8873 of LNCS, pages 158–178. Springer, 2014. 





SHZ+15. Bing Sun, Xin Hai, Wenyu Zhang, Lei Cheng, and Zhichao Yang. New observation on division property. IACR Cryptology ePrint Archive, 2015:459, 2015. 





Tod15a. Yosuke Todo. Integral cryptanalysis on full MISTY1. In Rosario Gennaro and Matthew Robshaw, editors, CRYPTO Part I, volume 9215 of LNCS, pages 413–432. Springer, 2015. 





Tod15b. Yosuke Todo. Structural evaluation by generalized integral property. In Elisabeth Oswald and Marc Fischlin, editors, EUROCRYPT Part I, volume 9056 of LNCS, pages 287–314. Springer, 2015. 





WLV+14. Qingju Wang, Zhiqiang Liu, Kerem Varici, Yu Sasaki, Vincent Rijmen, and Yosuke Todo. Cryptanalysis of reduced-round SIMON32 and SIMON48. In INDOCRYPT, volume 8885 of LNCS, pages 143–160. Springer, 2014. 





YZS+15. Gangqiang Yang, Bo Zhu, Valentin Suder, Mark D. Aagaard, and Guang Gong. The simeck family of lightweight block ciphers. In Tim G¨uneysu and Helena Handschuh, editors, CHES 2015, volume 9293 of LNCS, pages 307–329. Springer, 2015. 





ZW15. Huiling Zhang and Wenling Wu. Structural evaluation for generalized feistel structures and applications to lblock and TWINE. In Alex Biryukov and Vipul Goyal, editors, INDOCRYPT, volume 9462 of LNCS, pages 218–237. Springer, 2015. 



## A Propagation of Conventional Bit-Based Division Property on Toy Simon-Like Cipher

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/4f289e73-4343-47c0-b447-1a3f3cd1e7fd/94d56026382a7385b575e7ec484bfe8069cd2339e095ee23f2ed6d73b41358bd.jpg)



Fig. 3. Round function of the Simon-like ciphers


To help readers understand the propagation of the bit-based division property, we show the specific propagation on a Simon-like cipher. The block length of the Simon-like cipher is 8 bits, and $( L _ { i + 1 } , R _ { i + 1 } )$ is computed as 

$$
(L _ {i + 1}, R _ {i + 1}) = (L _ {i} ^ {\lll 1} \wedge L _ {i}) \oplus L _ {i} ^ {\lll 2} \oplus R _ {i} \oplus k _ {i}, L _ {i}).
$$

The left figure in Fig. 3 shows the round function. 

$\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 8 } }$ propagation on the equivalent circuit shown in the right figure in Fig. 3. Notice that we can remove XORing with the round key when the conventional bit-based division property is evaluated. 

As an example, let us consider $2 ^ { 3 }$ chosen plaintexts that all bits in $L _ { 0 }$ and the first one bit in $R _ { 0 }$ is constant and the others are active. Such set has the bit-based division $\mathcal { D } _ { \{ [ 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 ] \} } ^ { 1 ^ { 8 } }$ . Hereinafter, $[ 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 ]$ is referred to as [00000111] for the simplicity. 

Round 1 Now, the bit-based division property of $( L _ { 0 } , R _ { 0 } )$ is $\mathcal { D } _ { \{ [ 0 0 0 0 0 1 1 1 ] \} } ^ { 1 ^ { 8 } }$ . 

Step 1 Since the 8th bit of [00000111] is one, there is no change in K. 

Step 2 Since the 7th bit of [00000111] is one, there is no change in K. 

Step 3 Since the 6th bit of [00000111] is one, there is no change in K. 

Step 4 Since the 1st, 2nd, and 4th bits of [00000111] are zero, there is no change in K. 

$( L _ { 1 } , R _ { 1 } )$ $\mathcal { D } _ { \{ [ 0 1 1 1 0 0 0 0 ] \} } ^ { 1 ^ { 8 } }$ 

$( L _ { 1 } , R _ { 1 } )$ $\mathcal { D } _ { \{ [ 0 1 1 1 0 0 0 0 ] \} } ^ { 1 ^ { 8 } }$ 

Step 1 The propagation from [01110000] generates three vectors as 

$$
[ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 1 1 0 0 0 0 1 ], [ 0 0 1 1 0 0 0 1 ].
$$

Step 2 From every vector, the following six vectors 

$$
[ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 1 0 0 0 0 1 0 ],
$$

$$
[ 0 1 1 0 0 0 0 1 ] \Rightarrow [ 0 1 1 0 0 0 0 1 ], [ 0 1 0 0 0 0 1 1 ],
$$

$$
[ 0 0 1 1 0 0 0 1 ] \Rightarrow [ 0 0 1 1 0 0 0 1 ], [ 0 0 0 0 0 0 1 1 ],
$$

are generated. Here, [01000011] is redundant because $[ 0 1 0 0 0 0 1 1 ] \succeq [ 0 1 0 0 0 0 1 0 ]$ . 

Step 3 From every vector, the following 11 vectors 

$$
[ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 0 0 1 0 1 0 0 ], [ 0 1 1 0 0 1 0 0 ],
$$

$$
[ 0 1 0 0 0 0 1 0 ] \Rightarrow [ 0 1 0 0 0 0 1 0 ], [ 0 0 0 0 0 1 1 0 ],
$$

$$
[ 0 1 1 0 0 0 0 1 ] \Rightarrow [ 0 1 1 0 0 0 0 1 ], [ 0 0 0 0 0 1 0 1 ],
$$

$$
[ 0 0 1 1 0 0 0 1 ] \Rightarrow [ 0 0 1 1 0 0 0 1 ], [ 0 0 0 1 0 1 0 1 ], [ 0 0 1 0 0 1 0 1 ],
$$

$$
[ 0 0 0 0 0 0 1 1 ] \Rightarrow [ 0 0 0 0 0 0 1 1 ],
$$

are generated. Here, [00010101] is redundant because $[ 0 0 0 1 0 1 0 1 ] \succeq [ 0 0 0 1 0 1 0 0 ]$ . Moreover, [00100101] is redundant because $[ 0 0 1 0 0 1 0 1 ] \succeq [ 0 0 0 0 0 1 0 1 ]$ . 

Step 4 From every vector, the following 17 vectors 

$$
[ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 0 1 1 1 0 0 0 ], [ 0 1 0 1 1 0 0 0 ],
$$

$$
[ 0 0 0 1 0 1 0 0 ] \Rightarrow [ 0 0 0 1 0 1 0 0 ],
$$

$$
[ 0 1 1 0 0 1 0 0 ] \Rightarrow [ 0 1 1 0 0 1 0 0 ], [ 0 0 1 0 1 1 0 0 ], [ 0 1 0 0 1 1 0 0 ],
$$

$$
[ 0 1 0 0 0 0 1 0 ] \Rightarrow [ 0 1 0 0 0 0 1 0 ], [ 0 0 0 0 1 0 1 0 ],
$$

$$
[ 0 0 0 0 0 1 1 0 ] \Rightarrow [ 0 0 0 0 0 1 1 0 ],
$$

$$
[ 0 1 1 0 0 0 0 1 ] \Rightarrow [ 0 1 1 0 0 0 0 1 ], [ 0 0 1 0 1 0 0 1 ], [ 0 1 0 0 1 0 0 1 ],
$$

$$
[ 0 0 0 0 0 1 0 1 ] \Rightarrow [ 0 0 0 0 0 1 0 1 ],
$$

$$
[ 0 0 1 1 0 0 0 1 ] \Rightarrow [ 0 0 1 1 0 0 0 1 ], [ 0 0 0 1 1 0 0 1 ],
$$

$$
[ 0 0 0 0 0 0 1 1 ] \Rightarrow [ 0 0 0 0 0 0 1 1 ],
$$

are generated. 

After the swapping, the bit-based division property of $( L _ { 2 } , R _ { 2 } )$ is $\mathcal { D } _ { \mathbb { K } ^ { \prime } } ^ { 1 ^ { 8 } }$ , where $\mathbb { K } ^ { \prime }$ has 17 vectors. 

After the 2nd round, we similarly evaluate the propagation of the bit-based division prop-$( L _ { 4 } , R _ { 4 } )$ $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { 8 } }$ 

$$
\mathbb {K} = \{[ 0 0 0 1 0 0 0 0 ], [ 0 0 1 0 0 0 0 0 ], [ 0 1 0 0 0 0 0 0 ], [ 1 0 0 0 0 0 0 0 ], [ 0 0 0 0 0 0 1 0 ], [ 0 0 0 0 0 1 0 0 ], [ 0 0 0 0 1 0 0 1 ] \}.
$$

It means that the first one bit and the last one bit of $R _ { 4 }$ are balanced. 


Table 9. Propagation of the bit-based division property on Simon32


<table><tr><td>#rounds</td><td><eq>|\mathbb{K}|</eq></td><td><eq>\min_w(\mathbb{K})</eq></td><td><eq>\max_w(\mathbb{K})</eq></td></tr><tr><td>0 (plaintexts)</td><td>1</td><td>31</td><td>31</td></tr><tr><td>1</td><td>1</td><td>31</td><td>31</td></tr><tr><td>2</td><td>3</td><td>30</td><td>31</td></tr><tr><td>3</td><td>11</td><td>29</td><td>31</td></tr><tr><td>4</td><td>65</td><td>27</td><td>31</td></tr><tr><td>5</td><td>774</td><td>24</td><td>31</td></tr><tr><td>6</td><td>18165</td><td>19</td><td>31</td></tr><tr><td>7</td><td>587692</td><td>14</td><td>30</td></tr><tr><td>8</td><td>5191387</td><td>8</td><td>25</td></tr><tr><td>9</td><td>1595164</td><td>5</td><td>18</td></tr><tr><td>10</td><td>95768</td><td>3</td><td>14</td></tr><tr><td>11</td><td>5894</td><td>2</td><td>6</td></tr><tr><td>12</td><td>682</td><td>2</td><td>4</td></tr><tr><td>13</td><td>136</td><td>1</td><td>2</td></tr><tr><td>14</td><td>32</td><td>1</td><td>1</td></tr></table>

## B Propagation of Bit-Based Division Property on Simon32

We first get the 13-round integral characteristic on Simon32 from the propagation of the conventional bit-based division property. Table 9 shows the propagation characteristic, where there are not redundant vectors in K and $\mathrm { m i n } _ { w } ( \mathbb { K } )$ and $\operatorname* { m a x } _ { w } ( \mathbb { K } )$ are calculated as 

$$
\min _ {w} (\mathbb {K}) = \min _ {\boldsymbol {k} \in \mathbb {K}} \left(\sum_ {i = 1} ^ {3 2} w (k _ {i})\right), \max _ {w} (\mathbb {K}) = \max _ {\boldsymbol {k} \in \mathbb {K}} \left(\sum_ {i = 1} ^ {3 2} w (k _ {i})\right).
$$

Since round keys are XORed after the round function in Simon, we can trivially get the 14-round integral characteristic. Here, $2 ^ { 3 1 }$ plaintexts are chosen as $( L _ { 0 } , F ( L _ { 0 } ) \oplus R _ { 0 } )$ , where the first bit of $R _ { 0 }$ is constant and the others are active. 

## C Proof of Propagation of Bit-Based Division Property using Three Subsets

## C.1 Proof of Rule 1

Let F be a copy function, where the input $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ takes values of $\mathbb { F } _ { 2 } ^ { m }$ , and the output is calculated as $( x _ { 1 } , x _ { 1 } , x _ { 2 } , x _ { 3 } , \dots , x _ { m } )$ . Let X and Y be the input multiset and output multiset, respectively. Now, we want to evaluate the parity $\textcircled { \textregistered } _ { \boldsymbol { y } \in \mathbb { Y } } \pi _ { \boldsymbol { v } } ( \boldsymbol { y } )$ for any $\pmb { v } \in \mathbb { F } _ { 2 } ^ { m + 1 }$ 

$$
\begin{array}{l} \bigoplus_ {\boldsymbol {y} \in \mathbb {Y}} \pi_ {\boldsymbol {v}} (\boldsymbol {y}) = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (\pi_ {\boldsymbol {v}} \circ F) (\boldsymbol {x}) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} x _ {1} ^ {v _ {1}} x _ {1} ^ {v _ {2}} x _ {2} ^ {v _ {3}} x _ {3} ^ {v _ {4}} \dots x _ {m} ^ {v _ {m + 1}} \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ v _ {1} \vee v _ {2}, v _ {3}, \dots , v _ {m + 1} ]} (\boldsymbol {x}). \\ \end{array}
$$

$\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } } , \pi _ { \pmb { u } } ( \pmb { x } )$ 

$$
\bigoplus_ {x \in \mathbb {X}} \pi_ {\boldsymbol {u}} (x) = \left\{ \begin{array}{l l} \text {unknown} & \text {if there are \boldsymbol{k} \in \mathbb {K} s.t. \boldsymbol{u} \succeq \boldsymbol{k},} \\ 1 & \text {else if there is \boldsymbol{\ell} \in \mathbb {L} s.t. \boldsymbol{u} = \boldsymbol{\ell},} \\ 0 & \text {otherwise.} \end{array} \right.
$$

Therefore, $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ becomes unknown if and only if there are $k \in \mathbb { K }$ satisfying $W ( \boldsymbol { \mathrm { [ } } \boldsymbol { v _ { 1 } } \lor$ $v _ { 2 } , v _ { 3 } , \ldots , v _ { m + 1 } ] ) \succeq k$ . Thus, the parity becomes unknown for any v satisfying 

$$
W (\boldsymbol {v}) \succeq (1, 0, k _ {2}, k _ {3}, \dots , k _ {m}) \text { or } W (\boldsymbol {v}) \succeq (0, 1, k _ {2}, k _ {3}, \dots , k _ {m})
$$

for all $k \in \mathbb { K } \ \mathrm { s . t . } \ k _ { 1 } = 1$ . Moreover, the parity becomes unknown for any v satisfying 

$$
W (\boldsymbol {v}) \succeq (0, 0, k _ {2}, k _ {3}, \dots , k _ {m})
$$

for all $\pmb { k } \in \mathbb { K } \mathrm { ~ s . t . ~ } k _ { 1 } = 0$ . 

Next, if there are not $k \in \mathbb { K }$ satisfying $W ( [ v _ { 1 } \lor v _ { 2 } , v _ { 3 } , . . . , v _ { m + 1 } ] ) \ \succeq \ k$ , the parity $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ can exactly be determined. Then, the parity is 1 if there is $\ell \in \mathbb { L }$ satisfying $\bar { W } ( [ v _ { 1 } \lor v _ { 2 } , v _ { 3 } , . . . , v _ { m + 1 } ] ) = \ell .$ . Otherwise, the parity is 0. 

## C.2 Proof of Rule 2

Let F be a non-linear function, where the input $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ takes values of $( \mathbb { F } _ { 2 } ) ^ { m }$ , and the output is calculated as $( x _ { 1 } \wedge x _ { 2 } , x _ { 3 } , \dots , x _ { m } )$ . Let X and Y be the input multiset and output multiset, respectively. Now, we want to evaluate the parity $\textcircled { \textregistered } _ { \boldsymbol { y } \in \mathbb { Y } } \pi _ { \boldsymbol { v } } ( \boldsymbol { y } )$ for any $\pmb { v } \in \mathbb { F } _ { 2 } ^ { m - 1 }$ 

$$
\begin{array}{l} \bigoplus_ {\boldsymbol {y} \in \mathbb {Y}} \pi_ {\boldsymbol {v}} (\boldsymbol {y}) = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (\pi_ {\boldsymbol {v}} \circ F) (\boldsymbol {x}) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (x _ {1} x _ {2}) ^ {v _ {1}} x _ {3} ^ {v _ {2}} x _ {4} ^ {v _ {3}} \dots x _ {m} ^ {v _ {m - 1}} \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ v _ {1}, v _ {1}, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]} (\boldsymbol {x}). \\ \end{array}
$$

$\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } } , \pi _ { \pmb { u } } ( \pmb { x } )$ 

$$
\bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {\boldsymbol {u}} (x) = \left\{ \begin{array}{l l} \text {unknown} & \text {if there are \boldsymbol{k} \in \mathbb {K} s.t. \boldsymbol{u} \succeq \boldsymbol{k},} \\ 1 & \text {else if there is \boldsymbol{\ell} \in \mathbb {L} s.t. \boldsymbol{u} = \boldsymbol{\ell},} \\ 0 & \text {otherwise.} \end{array} \right.
$$

Therefore, $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ becomes unknown if and only if there is $k \in \mathbb { K }$ satisfying 

$$
W ([ v _ {1}, v _ {1}, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]) \succeq \boldsymbol {k}.
$$

It means that the parity becomes unknown for any v satisfying 

$$
W (\boldsymbol {v}) \succeq \left(\left\lceil \frac {k _ {1} + k _ {2}}{2} \right\rceil , k _ {3}, k _ {4}, \dots , k _ {m}\right).
$$

Next, if there are not $k \in \mathbb { K }$ satisfying $W ( [ v _ { 1 } , v _ { 1 } , v _ { 2 } , v _ { 3 } , \ldots , v _ { m - 1 } ] ) \ \succeq \ k$ , the parity $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ can exactly be determined. Then, the parity becomes 1 if there is $\ell \in \mathbb { L }$ satisfying $W ( [ v _ { 1 } , v _ { 1 } , v _ { 2 } , v _ { 3 } , \ldots , v _ { m - 1 } ] ) = \ell .$ Otherwise, the parity becomes 0. 

## C.3 Proof of Rule 3

Let F be a function compressed by an XOR, where the input $( x _ { 1 } , x _ { 2 } , \ldots , x _ { m } )$ takes values of $\mathbb { F } _ { 2 } ^ { m }$ , and the output is calculated as $( x _ { 1 } \oplus x _ { 2 } , x _ { 3 } , \ldots , x _ { m } )$ . Let X and Y be the input multiset and output multiset, respectively. Now, we want to evaluate the parity $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ for any $\pmb { v } \in \mathbb { F } _ { 2 } ^ { m - 1 }$ 

$$
\begin{array}{l} \bigoplus_ {\boldsymbol {y} \in \mathbb {Y}} \pi_ {\boldsymbol {v}} (\boldsymbol {y}) = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (\pi_ {\boldsymbol {v}} \circ F) (\boldsymbol {x}) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} (x _ {1} \oplus x _ {2}) ^ {v _ {1}} x _ {3} ^ {v _ {2}} x _ {4} ^ {v _ {3}} \dots x _ {m} ^ {v _ {m - 1}} \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} x _ {1} ^ {v _ {1}} x _ {3} ^ {v _ {2}} x _ {4} ^ {v _ {3}} \dots x _ {m} ^ {v _ {m - 1}} \oplus x _ {2} ^ {v _ {1}} x _ {3} ^ {v _ {2}} x _ {4} ^ {v _ {3}} \dots x _ {m} ^ {v _ {m - 1}} \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ v _ {1}, 0, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]} (\boldsymbol {x}) \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {[ 0, v _ {1}, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]} (\boldsymbol {x}). \\ \end{array}
$$

$\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { m } } , \pi _ { \pmb { u } } ( \pmb { x } )$ 

$$
\bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {\boldsymbol {u}} (x) = \left\{ \begin{array}{l l} \text {unknown} & \text {if there are \boldsymbol{k} \in \mathbb {K} s.t. \boldsymbol{u} \succeq \boldsymbol{k},} \\ 1 & \text {else if there is \boldsymbol{\ell} \in \mathbb {L} s.t. \boldsymbol{u} = \boldsymbol{\ell},} \\ 0 & \text {otherwise.} \end{array} \right.
$$

Therefore, $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ becomes unknown if and only if there are $k \in \mathbb { K }$ satisfying 

$$
W ([ v _ {1}, 0, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]) \succeq \boldsymbol {k} \text {   or   } W ([ 0, v _ {1}, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]) \succeq \boldsymbol {k}.
$$

It means that the parity becomes unknown for any v satisfying 

$$
W (\boldsymbol {v}) \succeq (k _ {1} + k _ {2}, k _ {3}, k _ {4}, \dots , k _ {m}),
$$

where $( k _ { 1 } , k _ { 2 } ) = ( 0 , 0 ) , ( 1 , 0 ) , \mathrm { o r } ( 0 , 1 )$ . From $k \in \mathbb { K }$ satisfying $( k _ { 1 } , k _ { 2 } ) = ( 1 , 1 )$ , we cannot get v that the parity becomes unknown. 

Next, if there is not $k \in \mathbb { K }$ satisfying 

$$
W ([ v _ {1}, 0, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]) \succeq \boldsymbol {k} \text {or} W ([ 0, v _ {1}, v _ {2}, v _ {3}, \dots , v _ {m - 1} ]) \succeq \boldsymbol {k},
$$

the parity $\oplus _ { y \in \mathbb { Y } } \pi _ { v } ( y )$ can exactly be determined. Let $\ell _ { 1 } = W ( [ v _ { 1 } , 0 , v _ { 2 } , v _ { 3 } , \ldots , v _ { m - 1 } ] )$ and $\ell _ { 2 } = W ( [ 0 , v _ { 1 } , v _ { 2 } , v _ { 3 } , \ldots , v _ { m - 1 } ] )$ . Then, if there is either $\ell _ { 1 }$ or $\ell _ { 2 }$ in L, the parity becomes 1. If there are both $\ell _ { 1 }$ and $\ell _ { 2 }$ in L, the parity becomes 0. Otherwise, the parity becomes 0. 

## D Example of Propagation of Bit-Based Division Property using Three Subsets

We show an example of the propagation of the bit-based division property using three subsets. Let $F$ be the core operation of the Simon family (see 4). Let $\mathbb { X }$ and $\mathbb { Y }$ be the input multiset and the output multiset, respectively. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-09/4f289e73-4343-47c0-b447-1a3f3cd1e7fd/3861449442fb65d859453dbec1aa46bb972baaea007dd02b8344b01827f01f08.jpg)



Fig. 4. Core operation of the Simon family


$\mathcal { D } _ { \phi , [ 1 , 1 , 1 , 0 ] } ^ { 1 ^ { 4 } }$ . We consider the intermediate state $\left[ y _ { 1 } , y _ { 2 } , y _ { 3 } , w _ { 1 } , w _ { 2 } , w _ { 3 } , x _ { 4 } \right]$ , and evaluate the division property. From the Rule 1, such state has $\mathcal { D } _ { \phi , \mathbb { L } } ^ { 1 ^ { 7 } } .$ φ,L, where L is represented as 

$$
\begin{array}{l} [ 1, 1, 1, 0, 0, 0, 0 ] [ 1, 1, 0, 0, 0, 1, 0 ] [ 1, 1, 1, 0, 0, 1, 0 ] [ 1, 0, 1, 0, 1, 0, 0 ], \\ [ 1, 0, 0, 0, 1, 1, 0 ] [ 1, 0, 1, 0, 1, 1, 0 ] [ 1, 1, 1, 0, 1, 0, 0 ] [ 1, 1, 0, 0, 1, 1, 0 ], \\ [ 1, 1, 1, 0, 1, 1, 0 ] [ 0, 1, 1, 1, 0, 0, 0 ] [ 0, 1, 0, 1, 0, 1, 0 ] [ 0, 1, 1, 1, 0, 1, 0 ], \\ [ 0, 0, 1, 1, 1, 0, 0 ] [ 0, 0, 0, 1, 1, 1, 0 ] [ 0, 0, 1, 1, 1, 1, 0 ] [ 0, 1, 1, 1, 1, 0, 0 ], \\ [ 0, 1, 0, 1, 1, 1, 0 ] [ 0, 1, 1, 1, 1, 1, 0 ] [ 1, 1, 1, 1, 0, 0, 0 ] [ 1, 1, 0, 1, 0, 1, 0 ], \\ [ 1, 1, 1, 1, 0, 1, 0 ] [ 1, 0, 1, 1, 1, 0, 0 ] [ 1, 0, 0, 1, 1, 1, 0 ] [ 1, 0, 1, 1, 1, 1, 0 ], \\ [ 1, 1, 1, 1, 1, 0, 0 ] [ 1, 1, 0, 1, 1, 1, 0 ] [ 1, 1, 1, 1, 1, 1, 0 ]. \\ \end{array}
$$

Next, we consider the intermediate state $\left[ y _ { 1 } , y _ { 2 } , y _ { 3 } , w _ { 4 } , w _ { 3 } , x _ { 4 } \right]$ , and evaluate the division $\mathcal { D } _ { \phi , \mathbb { L } ^ { \prime } } ^ { 1 ^ { 6 } }$ , where $\mathbb { L } ^ { \prime }$ is represented as 

$$
\begin{array}{l} [ 1, 1, 1, 0, 0, 0 ] [ 1, 1, 0, 0, 1, 0 ] [ 1, 1, 1, 0, 1, 0 ] [ 0, 0, 1, 1, 0, 0 ], \\ [ 0, 0, 0, 1, 1, 0 ] [ 0, 0, 1, 1, 1, 0 ] [ 0, 1, 1, 1, 0, 0 ] [ 0, 1, 0, 1, 1, 0 ], \\ [ 0, 1, 1, 1, 1, 0 ] [ 1, 0, 1, 1, 0, 0 ] [ 1, 0, 0, 1, 1, 0 ] [ 1, 0, 1, 1, 1, 0 ], \\ [ 1, 1, 1, 1, 0, 0 ] [ 1, 1, 0, 1, 1, 0 ] [ 1, 1, 1, 1, 1, 0 ]. \\ \end{array}
$$

Third, we consider the intermediate state $[ y _ { 1 } , y _ { 2 } , y _ { 3 } , w _ { 5 } , x _ { 4 } ]$ , and evaluate the division prop-$\mathcal { D } _ { \phi , \mathbb { L } ^ { \prime \prime } } ^ { 1 ^ { 5 } }$ φ,L00 , where $\mathbb { L } ^ { \prime \prime }$ is represented as 

$$
[ 1, 1, 1, 0, 0 ] [ 1, 1, 0, 1, 0 ] [ 0, 0, 1, 1, 0 ] [ 0, 1, 1, 1, 0 ] [ 1, 0, 1, 1, 0 ].
$$

Here, two [1, 1, 1, 1, 0] is propagated from [1, 1, 1, 0, 1, 0] and [1, 1, 1, 1, 0, 0], and [1, 1, 1, 1, 0] $\left[ y _ { 1 } , y _ { 2 } , y _ { 3 } , y _ { 4 } \right]$ $\stackrel { \cdot } { \mathcal { D } } _ { \phi , \mathbb { L } ^ { \prime \prime \prime } } ^ { 1 ^ { 5 } }$ D15φ,L000 , where L000 $\mathbb { L } ^ { \prime \prime \prime }$ is represented as 

$$
[ 1, 1, 1, 0 ] [ 1, 1, 0, 1 ] [ 0, 0, 1, 1 ] [ 0, 1, 1, 1 ] [ 1, 0, 1, 1 ].
$$

## E Propagation of Bit-Based Division Property Using Three Subsets on Toy Simon-Like Cipher

To help readers understand the propagation of the bit-based division property using three subsets, we revisit the toy Simon-like cipher shown in Appendix A (see Fig. 3). 

$\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { 8 } }$ . We evaluate the propagation on the equivalent circuit shown in the right figure in Fig. 3. 

As an example, let us consider $2 ^ { 3 }$ chosen plaintexts that all bits in $L _ { 0 }$ and the first one bit in $R _ { 0 }$ is constant and the others are active. Such set has the bit-based division $\mathcal { D } _ { \phi , \{ [ 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 ] \} } ^ { 1 ^ { 8 } }$ $[ 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 ]$ the simplicity. 

Round 1 Now, the bit-based division property of $( L _ { 0 } , R _ { 0 } )$ is $\mathcal { D } _ { \phi , } ^ { 1 ^ { 8 } }$ {[00000111]}. 

Step 1 Since the 8th bit of [00000111] is one, there is no change in L. Moreover, there is no vector in K. 

Step 2 Since the 7th bit of [00000111] is one, there is no change in L. Moreover, there is no vector in K. 

Step 3 Since the 6th bit of [00000111] is one, there is no change in L. Moreover, there is no vector in K. 

Step 4 Since the 1st, 2nd, and 4th bits of [00000111] are zero, there is no change in L. Moreover, there is no vector in K. 

Finally, since the round key is XORed with the right half, the parity of $\pi _ { \left[ 0 0 0 0 1 1 1 \right] } ( \pmb { x } )$ for all $\pmb { x } \in \mathbb { X }$ becomes unknown. After the swapping, the bit-based division property of (L1, R1) is D18{[11 $( L _ { 1 } , R _ { 1 } ) { \mathrm { ~ i s ~ } } { \mathcal { D } } _ { \{ [ 1 1 1 0 0 0 0 ] \} , \{ [ 0 1 1 1 0 0 0 0 ] \} } ^ { 1 ^ { 8 } } .$ 

$( L _ { 1 } , R _ { 1 } )$ $\mathcal { D } _ { \{ [ 1 }  ^ { 1 ^ { 8 } }$ 

Step 1 The propagation from $[ 1 1 1 1 0 0 0 0 ] \in \mathbb { K }$ generates three vectors as 

$$
[ 1 1 1 1 0 0 0 0 ] \Rightarrow [ 1 1 1 1 0 0 0 0 ], [ 0 1 1 0 0 0 0 1 ], [ 1 0 1 1 0 0 0 1 ].
$$

Moreover, the propagation from [01110000] ∈ L generates three vectors as 

$$
[ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 0 1 1 0 0 0 1 ], [ 0 1 1 1 0 0 0 1 ],
$$

where [01110001] is redundant because [01110001]  [01100001]. 

Step 2 From every vector in K, the following eight vectors 

$$
[ 1 1 1 1 0 0 0 0 ] \Rightarrow [ 1 1 1 1 0 0 0 0 ], [ 1 1 0 0 0 0 1 0 ], [ 0 1 1 1 0 0 1 0 ],
$$

$$
[ 0 1 1 0 0 0 0 1 ] \Rightarrow [ 0 1 1 0 0 0 0 1 ], [ 0 1 0 0 0 0 1 1 ],
$$

$$
[ 1 0 1 1 0 0 0 1 ] \Rightarrow [ 1 0 1 1 0 0 0 1 ], [ 1 0 0 0 0 0 1 1 ], [ 0 0 1 1 0 0 1 1 ],
$$

are generated. Moreover, from every vector in L, the following 10 vectors 

$$
[ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 1 0 0 0 0 1 0 ], [ 0 1 0 1 0 0 1 0 ], [ 0 1 1 0 0 0 1 0 ], [ 0 1 1 1 0 0 1 0 ],
$$

$$
[ 0 0 1 1 0 0 0 1 ] \Rightarrow [ 0 0 1 1 0 0 0 1 ], [ 0 0 0 0 0 0 1 1 ], [ 0 0 0 1 0 0 1 1 ], [ 0 0 1 0 0 0 1 1 ], [ 0 0 1 1 0 0 1 1 ],
$$

are generated. Here, [01110010] and [00110011] are redundant because there are the same vector in $\mathbb { K } ^ { \prime } .$ . 

Step 3 From every vector in K, the following 19 vectors 

$$
[ 1 1 1 1 0 0 0 0 ] \Rightarrow [ 1 1 1 1 0 0 0 0 ], [ 1 0 0 1 0 1 0 0 ], [ 1 1 1 0 0 1 0 0 ],
$$

$$
[ 1 1 0 0 0 0 1 0 ] \Rightarrow [ 1 1 0 0 0 0 1 0 ], [ 1 0 0 0 0 1 1 0 ],
$$

$$
[ 0 1 1 1 0 0 1 0 ] \Rightarrow [ 0 1 1 1 0 0 1 0 ], [ 0 0 0 1 0 1 1 0 ], [ 0 1 1 0 0 1 1 0 ],
$$

$$
[ 0 1 1 0 0 0 0 1 ] \Rightarrow [ 0 1 1 0 0 0 0 1 ], [ 0 0 0 0 0 1 0 1 ],
$$

$$
[ 0 1 0 0 0 0 1 1 ] \Rightarrow [ 0 1 0 0 0 0 1 1 ], [ 0 0 0 0 0 1 1 1 ],
$$

$$
[ 1 0 1 1 0 0 0 1 ] \Rightarrow [ 1 0 1 1 0 0 0 1 ], [ 1 0 0 1 0 1 0 1 ], [ 1 0 1 0 0 1 0 1 ],
$$

$$
[ 1 0 0 0 0 0 1 1 ] \Rightarrow [ 1 0 0 0 0 0 1 1 ],
$$

$$
[ 0 0 1 1 0 0 1 1 ] \Rightarrow [ 0 0 1 1 0 0 1 1 ], [ 0 0 0 1 0 1 1 1 ], [ 0 0 1 0 0 1 1 1 ],
$$

are generated. Here, [00000111], [10010101], [10100101], [00010111], and [00100111] are redundant. Moreover, from every vector in L, the following 22 vectors 

$$
\begin{array}{l} [ 0 1 1 1 0 0 0 0 ] \Rightarrow [ 0 1 1 1 0 0 0 0 ], [ 0 0 0 1 0 1 0 0 ], [ 0 1 0 1 0 1 0 0 ], [ 0 0 1 1 0 1 0 0 ], [ 0 1 1 0 0 1 0 0 ], \\ [ 0 1 0 0 0 0 1 0 ] \Rightarrow [ 0 1 0 0 0 0 1 0 ], \\ [ 0 1 0 1 0 0 1 0 ] \Rightarrow [ 0 1 0 1 0 0 1 0 ], [ 0 1 0 0 0 1 1 0 ], [ 0 1 0 1 0 1 1 0 ], \\ [ 0 1 1 0 0 0 1 0 ] \Rightarrow [ 0 1 1 0 0 0 1 0 ], [ 0 0 0 0 0 1 1 0 ], [ 0 1 0 0 0 1 1 0 ], [ 0 0 1 0 0 1 1 0 ], [ 0 1 1 0 0 1 1 0 ], \\ [ 0 0 1 1 0 0 0 1 ] \Rightarrow [ 0 0 1 1 0 0 0 1 ], [ 0 0 1 0 0 1 0 1 ], [ 0 0 1 1 0 1 0 1 ], \\ [ 0 0 0 0 0 0 1 1 ] \Rightarrow [ 0 0 0 0 0 0 1 1 ], \\ [ 0 0 0 1 0 0 1 1 ] \Rightarrow [ 0 0 0 1 0 0 1 1 ], [ 0 0 0 0 0 1 1 1 ], [ 0 0 0 1 0 1 1 1 ], \\ [ 0 0 1 0 0 0 1 1 ] \Rightarrow [ 0 0 1 0 0 0 1 1 ], \\ \end{array}
$$

are generated. where [01000110] is cancelled because it is propagated from [01010010] and [01100010] twice. Moreover, [01010110], [00100101], [00110101], [01100110], [00000111], and [00010111] are redundant. 

Step 4 Repeating the similar procedure, elements in K are represented as 

$$
\begin{array}{l} [ 1 1 1 1 0 0 0 0 ], [ 0 1 1 0 0 0 0 1 ], [ 1 0 1 1 0 0 0 1 ], [ 0 1 1 1 0 0 1 0 ], [ 1 1 0 0 0 0 1 0 ], [ 0 0 1 1 0 0 1 1 ], \\ [ 0 1 0 0 0 0 1 1 ], [ 1 0 0 0 0 0 1 1 ], [ 1 0 0 1 0 1 0 0 ], [ 1 1 1 0 0 1 0 0 ], [ 0 0 0 0 0 1 0 1 ], [ 0 0 0 1 0 1 1 0 ], \\ [ 0 1 1 0 0 1 1 0 ], [ 1 0 0 0 0 1 1 0 ], [ 0 0 1 1 1 0 0 0 ], [ 1 1 0 1 1 0 0 0 ], [ 0 0 1 0 1 0 0 1 ], [ 0 1 0 0 1 0 0 1 ], \\ [ 1 0 0 1 1 0 0 1 ], [ 0 0 0 0 1 0 1 0 ], [ 0 0 0 1 1 1 0 0 ], [ 0 0 1 0 1 1 0 0 ], [ 1 1 0 0 1 1 0 0 ]. \\ \end{array}
$$

Moreover, elements in L are represented as 

$$
\begin{array}{l} [ 0 1 1 1 0 0 0 0 ], [ 0 0 1 1 0 0 0 1 ], [ 0 1 0 0 0 0 1 0 ], [ 0 1 0 1 0 0 1 0 ], [ 0 1 1 0 0 0 1 0 ], [ 0 0 0 0 0 0 1 1 ], \\ [ 0 0 0 1 0 0 1 1 ], [ 0 0 1 0 0 0 1 1 ], [ 0 0 0 1 0 1 0 0 ], [ 0 0 1 1 0 1 0 0 ], [ 0 1 0 1 0 1 0 0 ], [ 0 1 1 0 0 1 0 0 ], \\ [ 0 0 0 0 0 1 1 0 ], [ 0 0 1 0 0 1 1 0 ], [ 0 1 0 1 1 0 0 0 ], [ 0 0 0 1 1 0 0 1 ], [ 0 1 0 0 1 1 0 0 ]. \\ \end{array}
$$

Finally, the round key is XORed with the right half. Some new vectors, which are generated from $\ell \in \mathbb { L }$ , are inserted into K. Moreover, some vectors in L becomes redundant because of the new vectors of K. As a result, we get 17 vectors in L and 29 vectors in K. 

After the 2nd round, we similarly evaluate the propagation of the bit-based division prop-$( L _ { 4 } , R _ { 4 } )$ $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { 8 } }$ 

$$
\begin{array}{l} \mathbb {K} = \{[ 0 0 0 1 0 0 0 0 ], [ 0 0 1 0 0 0 0 0 ], [ 0 1 0 0 0 0 0 0 ], [ 1 0 0 0 0 0 0 0 ], [ 0 0 0 0 0 0 1 1 ], \\ [ 0 0 0 0 0 1 0 1 ], [ 0 0 0 0 0 1 1 0 ], [ 0 0 0 0 1 0 0 1 ], [ 0 0 0 0 1 0 1 0 ], [ 0 0 0 0 1 1 0 0 ] \}, \\ \end{array}
$$

and 

$$
\mathbb {L} = \{[ 0 0 0 0 0 0 1 0 ], [ 0 0 0 0 0 1 0 0 ] \}.
$$

It means that the sum of $R _ { 4 }$ is 0x6. The conventional bit-based division property proves that the first and the last bits of $R _ { 4 }$ are balanced in Appendix A. By using the bit-based division property using three subsets, we additionally know that the second and the third bits of $R _ { 4 }$ are always one. 


Table 10. Propagation of the bit-based division property using three subsets on Simon32


<table><tr><td>#rounds</td><td><eq>|\mathbb{K}|</eq></td><td><eq>\min_w(\mathbb{K})</eq></td><td><eq>\max_w(\mathbb{K})</eq></td><td><eq>|\mathbb{L}|</eq></td><td><eq>\min_w(\mathbb{L})</eq></td><td><eq>\max_w(\mathbb{L})</eq></td></tr><tr><td>0 (plaintexts)</td><td>1</td><td>32</td><td>32</td><td>1</td><td>31</td><td>31</td></tr><tr><td>1</td><td>1</td><td>32</td><td>32</td><td>1</td><td>31</td><td>31</td></tr><tr><td>2</td><td>1</td><td>32</td><td>32</td><td>5</td><td>30</td><td>31</td></tr><tr><td>3</td><td>6</td><td>30</td><td>31</td><td>19</td><td>29</td><td>31</td></tr><tr><td>4</td><td>43</td><td>28</td><td>30</td><td>138</td><td>27</td><td>31</td></tr><tr><td>5</td><td>722</td><td>25</td><td>29</td><td>2236</td><td>24</td><td>31</td></tr><tr><td>6</td><td>23321</td><td>20</td><td>29</td><td>89878</td><td>19</td><td>31</td></tr><tr><td>7</td><td>996837</td><td>15</td><td>28</td><td>4485379</td><td>14</td><td>30</td></tr><tr><td>8</td><td>9849735</td><td>8</td><td>26</td><td>47149981</td><td>8</td><td>25</td></tr><tr><td>9</td><td>2524718</td><td>5</td><td>19</td><td>2453101</td><td>5</td><td>18</td></tr><tr><td>10</td><td>130724</td><td>3</td><td>14</td><td>20360</td><td>3</td><td>13</td></tr><tr><td>11</td><td>7483</td><td>2</td><td>7</td><td>168</td><td>2</td><td>6</td></tr><tr><td>12</td><td>852</td><td>2</td><td>4</td><td>8</td><td>2</td><td>3</td></tr><tr><td>13</td><td>181</td><td>1</td><td>2</td><td>0</td><td>-</td><td>-</td></tr><tr><td>14</td><td>32</td><td>1</td><td>2</td><td>0</td><td>-</td><td>-</td></tr><tr><td>15</td><td>32</td><td>1</td><td>1</td><td>0</td><td>-</td><td>-</td></tr></table>

## F Propagation of Bit-Based Division Property using Three Subsets on Simon32

We first get the 14-round integral characteristic on Simon32 from the propagation of the bitbased division property using three subsets. Table 10 shows the propagation characteristic, where there are not redundant vectors in K and $\mathbb { L } ,$ and $\mathrm { m i n } _ { w } ( \mathbb { K } )$ , ma $\mathrm { x } _ { w } ( \mathbb { K } )$ , $\mathrm { m i n } _ { w } ( \mathbb { L } )$ , and $\operatorname* { m a x } _ { w } ( \mathbb { L } )$ are calculated as 

$$
\min _ {w} (\mathbb {K}) = \min _ {\boldsymbol {k} \in \mathbb {K}} \left(\sum_ {i = 1} ^ {3 2} w (k _ {i})\right), \max _ {w} (\mathbb {K}) = \max _ {\boldsymbol {k} \in \mathbb {K}} \left(\sum_ {i = 1} ^ {3 2} w (k _ {i})\right),
$$

$$
\min _ {w} (\mathbb {L}) = \min _ {\boldsymbol {\ell} \in \mathbb {L}} \left(\sum_ {i = 1} ^ {3 2} w (\ell_ {i})\right), \max _ {w} (\mathbb {L}) = \max _ {\boldsymbol {\ell} \in \mathbb {L}} \left(\sum_ {i = 1} ^ {3 2} w (\ell_ {i})\right).
$$

Since round keys are XORed after the round function in Simon, we can trivially get the 15-round integral characteristic. Here, $2 ^ { 3 1 }$ plaintexts are chosen as $( L _ { 0 } , F ( L _ { 0 } ) \oplus R _ { 0 } )$ , where the first bit of $R _ { 0 }$ is constant and the others are active. 

Table 10 shows that the output of the 12th round function has the division property D132K L, $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { 3 2 } }$ where $| \mathbb { K } | = 8 5 2$ and $| \mathbb { L } | = 8$ . Here, eight vectors in L are represented by hexadecimal notation as 

$\begin{array} { r l r l r l } { ( 0 0 0 4 0 0 0 1 ) } & { ( 0 0 0 4 0 0 0 3 ) } & { ( 0 0 0 4 0 4 0 1 ) } & { ( 0 0 0 4 0 4 0 2 ) } & { ( 0 0 0 0 0 4 0 3 ) } & { ( 0 0 0 0 0 4 4 0 ) } & { ( 0 0 0 0 0 4 4 0 ) } & { ( 0 0 0 0 1 0 0 1 ) } & { ( 0 0 0 0 2 1 0 0 ) . } \end{array}$ 

We experimentally confirmed whether or not the parity is 1 by randomly choosing the secret key, and we repeat the confirmation 20 times. As a result, we confirmed that the parity is always 1. 