# EasyBC: A Cryptography-Specific Language for Security Analysis of Block Ciphers against Differential Cryptanalysis

PU SUN, ShanghaiTech University, China 

FU SONG∗, Institute of Software at Chinese Academy of Sciences, China and University of Chinese Academy of Sciences, China 

YUQI CHEN, ShanghaiTech University, China 

TAOLUE CHEN, Birkbeck, University of London, UK 

Differential cryptanalysis is a powerful algorithmic-level attack, playing a central role in evaluating the security of symmetric cryptographic primitives. In general, the resistance against differential cryptanalysis can be characterized by the maximum expected differential characteristic probability. In this paper, we present generic and extensible approaches based on mixed integer linear programming (MILP) to bound such probability. We design a high-level cryptography-specific language EasyBC tailored for block ciphers and provide various rigorous procedures as differential denotational semantics, to automate the generation of MILP from block ciphers written in EasyBC. We implement an open-sourced tool that provides support for fully automated resistance evaluation of block ciphers against differential cryptanalysis. The tool is extensively evaluated on 23 real-life cryptographic primitives including all the 10 finalists of the NIST lightweight cryptography standardization process. The experiments confirm the expressivity of EasyBC and show that the tool can effectively prove the resistance against differential cryptanalysis for all block ciphers under consideration. EasyBC makes resistance evaluation against differential cryptanalysis easily accessible to cryptographers. 

CCS Concepts: • Theory of computation; • Security and privacy; 

Additional Key Words and Phrases: Cryptography-Specific Language, Block Ciphers, Differential Cryptanalysis 

## ACM Reference Format:

Pu Sun, Fu Song, Yuqi Chen, and Taolue Chen. 2024. EasyBC: A Cryptography-Specific Language for Security Analysis of Block Ciphers against Differential Cryptanalysis. Proc. ACM Program. Lang. 8, POPL, Article 29 (January 2024), 49 pages. https://doi.org/10.1145/3632871 

## 1 INTRODUCTION

A block cipher is a symmetric cryptographic technique that uses the same key to encrypt and decrypt data in fixed-size blocks. Guided by the design principles of block ciphers [Shannon 1949], a vast number of block ciphers have been proposed, varying in, e.g., network structures and block sizes. Many of them have been standardized and are widely used in daily life to provide confidentiality, integrity, and authentication. Differential cryptanalysis, proposed by [Biham and Shamir 1990], is a powerful algorithmic-level attack against block ciphers by analyzing the effect of particular differences in input pairs on the differences of pairs of intermediate states under 

∗Corresponding author 

Authors’ addresses: Pu Sun, ShanghaiTech University, Shanghai, China, sunpu@shanghaitech.edu.cn; Fu Song, Institute of Software at Chinese Academy of Sciences, Beijing, China and University of Chinese Academy of Sciences, Beijing, China, songfu@ios.ac.cn; Yuqi Chen, ShanghaiTech University, Shanghai, China, chenyq@shanghaitech.edu.cn; Taolue Chen, Birkbeck, University of London, London, UK, t.chen@bbk.ac.uk. 

Permission to make digital or hard copies of part or all of this work for personal or classroom use is granted without fee provided that copies are not made or distributed for profit or commercial advantage and that copies bear this notice and the full citation on the first page. Copyrights for third-party components of this work must be honored. For all other uses, contact the owner/author(s). 

© 2024 Copyright held by the owner/author(s). 

ACM 2475-1421/2024/1-ART29 

https://doi.org/10.1145/3632871 

the same key. It has proved to be a very effective attack, which has broken block ciphers such as DES [Biham and Shamir 1990], FEAL [Aoki et al. 1997], WARP [Teh and Biryukov 2022], and K-Cipher [Mahzoun et al. 2022]. As a result, a provable guarantee of the resistance of block ciphers against differential cryptanalysis has become a standard criterion for new block ciphers and indeed a basic requirement for them to be standardized [Katz and Lindell 2014]. In light of the diversity and wide deployment of block ciphers, a generic, and ideally automated, approach which can be used to evaluate their resistance against differential cryptanalysis, becomes indispensable. 

In general, the resistance of block ciphers against differential cryptanalysis is commonly characterized by the maximum expected differential characteristic probability (MaxEDCP for short, the definition of which is fairly standard but technical, and will be given in Section 2.1). A block cipher is considered to be resistant against differential cryptanalysis if the MaxEDCP is no greater than 2−??, where ?? is the block size of the block cipher [Heys 2002a; Lai et al. 1991]. In light of this, the central task of security analysis for block ciphers against differential cryptanalysis is reduced to computing such probability. To this end, [Matsui 1994] proposed a branch-and-bound searching algorithm that traverses differential characteristics in a depth-first manner and computes their probabilities during traversal. However, it becomes inefficient with the increasing of candidate differential characteristics. Various heuristics are then proposed to improve the efficiency [Aoki et al. 1997; Bao et al. 2014; Biryukov and Nikolić 2010; Ji et al. 2021], but they harness cipher-specific optimizations and thus require sophisticated programming skills. 

Several alternative methods are introduced, which reduce to mixed integer linear program (MILP [Mouha et al. 2011]), Boolean satisfiability problem (SAT [Mouha and Preneel 2013]), or satisfiability modulo theory (SMT [Aumasson et al. 2014]). These methods allow cryptanalysts to specify the problem for each cipher using the input language of MILP/SAT/SMT solvers so that respective solvers can be harnessed. Plenty of modeling methods for cryptography-specific operations such as substitution-box (S-box), Exclusive-OR (XOR), and linear transformations, as well as heuristics, are proposed to improve efficiency and accuracy. However, insofar cryptanalysts have to manually model each cipher in the tool they choose to use, or at best write a model generation script for each cipher, which is usually intricate, error-prone, and laborious, as it requires the cryptanalysts to be familiar with the specific tool and a wide range of modeling methods. There appears to be a lack of language support, unified computational approaches, and fully automated tools for evaluating the resistance of block ciphers against differential cryptanalysis. 

Contributions. In this work, our primary aim is to develop a generic and automated approach for evaluating the resistance of block ciphers against differential cryptanalysis. To achieve this goal, we begin by designing a novel high-level statically-typed, C-like cryptography-specific language, EasyBC (Easy Block Cipher), tailored for block ciphers. Besides standard types and operations, EasyBC provides cryptography-specific types (e.g., S-boxes, P-boxes) and operations (e.g., substitution via S-box, linear transformation via P-box, or matrix-vector product), to facilitate the implementation of block ciphers. (Note that an S-box takes ?? input bits and transforms them into ?? output bits and P-box, short for Permutation-box, is an array specifying a permutation of inputs.) The language is fully specified by a formal grammar together with typing rules and operational semantics, enabling further automatic analysis. 

Concretely speaking, the analysis of block cipher resistance against differential cryptanalysis primarily involves computing the MaxEDCP. However, calculating MaxEDCP precisely is practically infeasible. As a result, we employ a strategy to calculate a tight upper bound that is sufficient to demonstrate resistance. Specifically, we adopt the typical MILP-based approach where linear constraints are used to characterize the dependency (i.e., feasibility) between input and output differences of each operation, and the optimization objective is to minimize an upper bound. In particular, we give a rigorous procedure, formalized as a differential denotational semantics, to automate the generation of MILP from EasyBC programs, which not only unifies and optimizes the existing but also discovers new generation processes. Our approach is of generic nature, thanks to the expressivity of the EasyBC langauge, namely, a multitude of block ciphers can be handled in a unified way. 

Technically, the generation of MILP can be done either at the word or bit level. The former is less involved and generates fewer constraints, but is limited to certain block ciphers; the latter approach is more fine-grained and has wider applicability. In both cases, the general strategy is to determine the lower bound of the minimum number of (differentially) active S-boxes, i.e., S-boxes whose input differences are nonzero under two executions [Biryukov and Nikolić 2010; Heys 2002b], from which the upper bound of the MaxEDCP can be deduced according to [Heys 2002b; Sun et al. 2014a]. While this strategy is efficient, it is important to note that the obtained upper bound may not always be sufficiently tight and may not be applicable for certain ciphers. To address this limitation, we introduce an extended bit-wise approach that directly bounds the MaxEDCP by encoding probabilities using additional Boolean variables. We have successfully implemented our approach as the first fully automated tool for evaluating the resistance of block ciphers against differential cryptanalysis. This tool eliminates the need for cryptanalysts to possess knowledge of MILP generation for cryptographic operations. Instead, they can simply write a program in EasyBC for a block cipher. Moreover, the generation of MILP from EasyBC program is modular, i.e., each cryptographic operation in EasyBC is associated with its own MILP generation rule and new generation rules could be easily added by implementing designated APIs. As a result, our approach exhibits excellent extensibility for new block ciphers, enabling a wider range of applicability. 

To evaluate the tool, we implement 23 realistic cryptographic primitives with EasyBC, including all the 10 finalists of the NIST lightweight cryptography standardization process [NIST 2023] and other commonly used block ciphers, covering both substitution-permutation network (SPN) based ciphers (e.g., AES, PRESENT, and GIFT) and balanced Feistel networks (BFN) based ciphers (e.g., DES, LBLOCK, and TWINE). It turns out that EasyBC can express these block ciphers in a considerably more succinct way, demonstrating our language’s expressiveness. Moreover, it turns out that our tool is able to effectively handle all realistic block ciphers under consideration, showcasing its capability in proving the resistance of block ciphers against differential cryptanalysis. 

In addition, we compare various alternative MILP generation methods for cryptographic operations. Interestingly, we observe that certain methods may generate fewer constraints and variables. However, it is worth noting that such reductions may actually have a negative impact on the overall MILP-solving process. For instance, the recent S-box modeling method proposed by [Udovenko 2021] produces the fewest constraints, but also exhibits the least performance to solve those constraints. (Overall, it is significantly less efficient than some alternatives.) Our findings shed light on the selection of generation methods among various alternatives for practical applications. 

We summarize the main contributions as follows. 

• We design a high-level cryptography-specific language EasyBC tailored for block ciphers, enabling further automatic analysis. 

• We give generic and extensible approaches for automated resistance evaluation of block ciphers written in EasyBC against differential cryptanalysis. 

• We implement and extensively evaluate an open-sourced prototype of EasyBC, which confirms the expressiveness of EasyBC and the effectiveness of our approach. 

Structure. The rest of the paper is organized as follows. Section 2 presents the background of block ciphers and differential cryptanalysis. Section 3 introduces EasyBC and an overview of our approach. Section 4 presents three key utilities used in our MILP generation. Section 5 and Section 6 describe the word-wise and bit-wise approach. Section 7 describes the extended bit-wise approach. Section 8 reports the experimental results. We discuss related work in Section 9 and conclude this work in Section 10. 

## 2 BACKGROUND

Throughout this paper, B denotes the Boolean domain $\{ 0 , 1 \}$ , and N denotes the set of non-negative integers. Boolean values are treated as integers in arithmetic computations. Given a vector/array ${ \vec { x } } ,$ , $\vec { x } _ { i }$ denotes the (?? + 1)-th entry. Given a matrix $M , M _ { i }$ denotes the (?? + 1)-th row, and $M _ { i , j }$ denotes the $( j + 1 )$ -th entry of ???? . An ??-bitstream is Boolean vector $\vec { b }$ with $m$ entries. We denote by ⊕ the bit-wise XOR operator. $\vec { b } \vert \vert \vec { b } ^ { \prime } = ( b _ { 0 } , \cdot \cdot \cdot , b _ { m } , b _ { 0 } ^ { \prime } , \cdot \cdot \cdot , b _ { n } ^ { \prime } )$ is the concatenation of two bitstreams $\vec { b } = ( b _ { 0 } , \cdots , b _ { m } )$ and $\vec { b } ^ { \prime } = ( b _ { 0 } ^ { \prime } , \cdot \cdot \cdot , b _ { n } ^ { \prime } )$ . We denote by bin(??) the binary representation of an unsigned integer ?? as a bitstream. 

## 2.1 Block ciphers

Block ciphers are a type of symmetric cryptography, which encrypts and decrypts data in fixed-size (e.g., 64 or 128 bits) blocks using the same key [Bogdanov 2010; Knudsen 1998]. 

Definition 2.1. A block cipher is a function Enc : $\mathbb { B } ^ { \mathcal { k } } \times \mathbb { B } ^ { \mathcal { k } } \to \mathbb { B } ^ { \mathcal { k } }$ such that for every key $K \in \mathbb { B } ^ { \mathcal { R } }$ , Enc $( K , \cdot )$ is a bijective function, where $\mathcal { \ell }$ is the block size and ?? is the key size. 

Intuitively, Enc $( K , \cdot )$ is a keyed-permutation that maps an input block to an output block, where a block is a ??-bitstream, the key ?? determines which permutation to perform. The input and output of $\mathsf { E n c } ( K , \cdot )$ are called plaintext and ciphertext, respectively. A block cipher Enc is ideal if it is defined by assigning a uniformly drawn permutation to each of the $2 ^ { \mathcal { k } }$ keyed-permutations. An ideal block cipher is commonly considered to be computationally secure if the key size $\mathscr { k }$ is large enough since the brute-force attack requires $O ( 2 ^ { \mathscr { k } } )$ time. However, it is extremely difficult to implement an ideal block cipher for practical block sizes (e.g., 64 or 128), as one randomly drawn permutation Enc $( K , \cdot )$ has to be stored for each given key ??. 

To be efficient yet strong, modern block ciphers apply several (possibly distinct) keyed permutations, where one keyed permutation is a round and implemented by a round function. 

Definition 2.2. A ??-round iterative block cipher (??-IBC) is a function Enc : $: \mathbb { B } ^ { \mathcal { k } } \times \mathbb { B } ^ { \mathcal { k } } \to \mathbb { B } ^ { \mathcal { k } }$ such that for every key $K \in \mathbb { B } ^ { \mathcal { R } }$ , 

$$
\mathsf {E n c} (K, \cdot) = \mathsf {E n c} _ {\nu} (K ^ {\nu}, \cdot) \circ \dots \circ \mathsf {E n c} _ {1} (K ^ {1}, \cdot),
$$

where Enc?? $: \mathbb { B } ^ { \mathcal { k } } \times \mathbb { B } ^ { \mathcal { \ell } } \to \mathbb { B } ^ { \mathcal { \ell } }$ is the ??-th round with its subkey $K ^ { i }$ for $1 \leq i \leq \varkappa$ , symbol ◦ denotes function composition, and the subkeys are generated via a key schedule algorithm $\begin{array} { r } { g : \mathbb { B } ^ { \not \mathcal { k } } \to ( \mathbb { B } ^ { \not \mathcal { k } } ) ^ { \not \prime } } \end{array}$ , $\operatorname { i . e . , } g ( K ) = \left( K ^ { 1 } , \cdots , K ^ { \ast } \right)$ . 

Given a key $K \in \mathbb { B } ^ { \mathcal { R } }$ , Enc $( K , \cdot )$ is used for encryption and $\mathsf { D e c } ( K , \cdot )$ is used for decryption, i.e., $\mathsf { D e c } ( K , \cdot ) = \mathsf { E n c } _ { 1 } ^ { - 1 } ( K ^ { 1 } , \cdot ) \circ \cdots \circ \mathsf { E n c } _ { \neq } ^ { - 1 } ( K ^ { \prime } , \cdot )$ , where $K ^ { i }$ is the ??-th round subkey in Definition 2.2. 

Block ciphers can be built in various ways following iterative cipher schemes. The two most widely used are substitution-permutation networks (SPN) [Kam and Davida 1979] and balanced Feistel networks (BFN) [Nyberg 1996]. Note that BFN is used in the former U.S. encryption standard (DES-Data Encryption Standard) [Fox 2000] and SPN is used in the current one (AES-Advanced Encryption Standard) [Daemen and Rijmen 1999]. A brief introduction of BFN and SPN is given in [Sun et al. 2023, Section $\mathrm { A l }$ . 

In the sequel, we fix a ??-IBC Enc : $\mathbb { B } ^ { \mathcal { k } } \times \mathbb { B } ^ { \mathcal { k } } \to \mathbb { B } ^ { \mathcal { k } }$ with rounds $\mathsf { E n c } _ { 1 } , \cdots , \mathsf { E n c } _ { \scriptscriptstyle r }$ . We assume that the attacker knows all dethe tails of the encryption and decryption except for the secret key. 

## 2.2 Differential Cryptanalysis

Differential cryptanalysis recovers the secret key by exploiting the fact that the probability of some output differences of rounds in a non-ideal block cipher is higher than the expected value $( \mathrm { i . e . , } 2 ^ { - \ell } )$ for certain input differences [Biham and Shamir 1990]. We review the related concepts below. 

Difference and differential. Given two ??-bitstreams $X \in \mathbb { B } ^ { \mathcal { Q } }$ and $X ^ { \prime } \in \mathbb { B } ^ { \mathcal { Q } }$ , their (XOR-) difference Δ?? is defined by $\Delta X = X \oplus X ^ { \prime }$ . Note that for any fixed difference $\Delta X$ , there are exactly $2 ^ { \mathcal { k } }$ pairs $( X , X ^ { \prime } )$ such that ?? ⊕ $X ^ { \prime } = \Delta X$ . A differential is defined to be a pair of differences (Δ??, Δ?? ). 

Given a pair of inputs $( X , X ^ { \prime } ) \in \mathbb { B } ^ { n } \times \mathbb { B } ^ { n }$ for a (deterministic) function $f : \mathbb { B } ^ { n }  \mathbb { B } ^ { m }$ , the input difference of the function ?? is the difference $\Delta X = X \oplus X ^ { \prime }$ of the inputs $( X , X ^ { \prime } )$ , and the output difference of ?? is the difference $f ( X ) \oplus f ( X ^ { \prime } )$ of the outputs $( f ( X ) , f ( X ^ { \prime } ) )$ . Clearly, when the input difference is fixed to be Δ?? , the output difference of an input ?? is $f ( X ) \oplus f ( X \oplus \Delta X )$ . 

The probability $\mathsf { P r } _ { f } ( \Delta X , \Delta Y )$ of a given differential $( \Delta X , \Delta Y )$ for the function ?? is the proportion of inputs $X \in \mathbb { B } ^ { n }$ such that the output difference $f ( X ) \oplus f ( X \oplus \Delta X )$ is equal to Δ?? , i.e., 

$$
\mathsf {P r} _ {f} (\Delta X, \Delta Y) = \frac {| \{X \in \mathbb {B} ^ {n} \mid f (X) \oplus f (X \oplus \Delta X) = \Delta Y \} |}{2 ^ {n}}.
$$

In particular, for an ??-th round $\mathsf { E n c } _ { i } : \mathbb { B } ^ { \not \star } \times \mathbb { B } ^ { \not \in }  \mathbb { B } ^ { \not \in }$ , with fixed subkey $K ^ { i } , \mathsf { P r } _ { \mathsf { E n c } _ { i } ( K ^ { i } , \cdot ) } ( \Delta X ^ { i - 1 } , \Delta X ^ { i } )$ is the probability of a differential $( \Delta X ^ { i - 1 } , \Delta X ^ { i } )$ for the function $\mathsf { E n c } _ { i } ( K ^ { i } , \cdot )$ . 

It is worth mentioning that in differential cryptanalysis, a key assumption is that the output difference $\mathsf { E n c } _ { i } ( K ^ { i } , X ^ { i - 1 } )$ ⊕ $\mathsf { E n c } _ { i } ( K ^ { i } , X ^ { i - 1 } \oplus \Delta X ^ { i - 1 } )$ of the ??-th round is independent of the subkey $K ^ { i }$ for any fixed input $X ^ { i - 1 }$ and input difference $\Delta X ^ { i - 1 }$ . As a result, for clarity, $\mathsf { P r } _ { \mathsf { E n c } _ { i } ( K ^ { i } , \cdot ) } ( \cdot )$ is simply written as $\mathsf { P r } _ { \mathsf { E n c } _ { i } } ( \cdot )$ . 

Differential characteristic. An ??-round differential characteristic is a vector $( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ of differences, where 

• $\Delta X ^ { 0 }$ a nonzero input difference to the cipher Enc : $\mathbb { B } ^ { \mathcal { k } } \times \mathbb { B } ^ { \mathcal { k } } \to \mathbb { B } ^ { \mathcal { k } }$ 

• for $1 \leq i \leq s , ( \Delta X ^ { i - 1 } , \Delta X ^ { i } )$ is a differential of the ??-th round $\mathsf { E n c } _ { i } ( K ^ { i } , \cdot )$ . 

Definition 2.3. The differential characteristic probability $\mathsf { P r } _ { \mathsf { E n c } ( K , \cdot ) } ( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ of an ??-round differential characteristic $( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ is the proportion of inputs $X ^ { 0 } \in \mathbb { B } ^ { \ell }$ to the cipher Enc such that the output difference of the ??-th round Enc?? is $\Delta X ^ { i }$ for every $1 \ \leq \ i \ \leq \ s$ in the two executions of Enc under the two inputs $( K , X ^ { 0 } )$ and $( K , X ^ { 0 } \oplus \Delta X ^ { 0 } )$ , namely, 

$$
\operatorname * {P r} _ {\operatorname{Enc} (K, \cdot)} (\Delta X ^ {0}, \dots , \Delta X ^ {s}) = \frac {\left| \left\{X ^ {0} \in \mathbb {B} ^ {\ell^ {\prime}} \mid \forall i . 1 \leq i \leq s . \operatorname{Enc} _ {\leq i} (X _ {0}) \oplus \operatorname{Enc} _ {\leq i} (X _ {0} \oplus \Delta X ^ {0}) = \Delta X ^ {i} \right\} \right|}{2 ^ {\ell^ {\prime}}}
$$

$\mathrm { w h e r e } \mathsf { E n c } _ { \le i } : = \mathsf { E n c } _ { i } ( K ^ { i } , \cdot ) \circ \cdot \cdot \cdot \circ \mathsf { E n c } _ { 1 } ( K ^ { 1 } , \cdot ) \mathrm { f o r } 1 \le i \le s .$ 

Recall that we assumed that the output difference En $\mathsf { c } _ { i } ( K ^ { i } , X ^ { i - 1 } )$ ⊕ Enc $\therefore ( K ^ { i } , X ^ { i - 1 } \oplus \Delta X ^ { i - 1 } )$ of the ??-th round is independent upon the subkey $K ^ { i }$ for any fixed input $X ^ { i - 1 }$ and input difference $\Delta X ^ { i - 1 }$ . Thus, the probability $\mathsf { P r } _ { \mathsf { E n c } ( K , \cdot ) } \left( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } \right)$ does not depend on the ??-round subkey $K ^ { s }$ , but depends on the other subkeys $K ^ { 1 } , \cdots , K ^ { s - 1 }$ . It is known that $\mathsf { P r } _ { \mathsf { E n c } ( K , \cdot ) } ( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ is upper bound by $\begin{array} { r } { \prod _ { i = 1 } ^ { s } \mathsf { P r } _ { \mathsf { E n c } _ { i } } ( \Delta X ^ { i - 1 } , \Delta X ^ { i } ) } \end{array}$ [Heys and Tavares 1996], and they are the same if Enc is a Markov cipher and its round subkeys are independent [Lai et al. 1991]. 

In a resistant block cipher, for any fixed key ??, the probability $\mathsf { P r } _ { \mathsf { E n c } ( K , \cdot ) } ( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ should be small enough for any differential characteristic $( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ if the input $X ^ { 0 }$ is sampled uniformly. However, in practice, $\mathsf { P r } _ { \mathsf { E n c } ( K , \cdot ) } ( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ may be higher than $2 ^ { - \bar { \alpha } }$ based on which an attacker can efficiently recover the subkey $K ^ { s + 1 }$ . The differential characteristic $( \widetilde { \Delta X } ^ { 0 } , \cdots , \widetilde { \Delta X } ^ { s } )$ is said to be optimal if it attains the greatest expected differential characteristic probability among all the ??- round differential characteristics. We remark that optimal differential characteristics are commonly assumed to be identical for different keys ?? during the attack, as the actual key is unknown to the adversary before attacking. In the sequel, for simplicity, $\mathsf { P r } _ { \mathsf { E n c } ( K , \cdot ) } ( \cdot )$ is written as $\mathsf { P r } _ { \mathtt { E n c } } ( \cdot )$ and $\mathsf { P r } _ { \mathsf { E n c } } ( \widetilde { \Delta X } ^ { 0 } , \cdot \cdot \cdot , \widetilde { \Delta X } ^ { s } )$ is refer to as the maximum expected differential characteristic probability (MaxEDCP). The key recovering procedure is given in [Sun et al. 2023, Section B], where the number of plaintexts required to infer the (?? +1)-round subkey $K ^ { s + 1 }$ is proportional to $\frac { 1 } { \mathsf { P r } _ { \mathsf { E n c } } ( \widetilde { \Delta X } ^ { 0 } , \cdots , \widetilde { \Delta X } ^ { s } ) }$ [Heys g g2002b], i.e., the reciprocal of the MaxEDCP of ??-round differential characteristics. As a result, if one could show that an upper bound of $\mathsf { P r } _ { \mathsf { E n c } } ( \widetilde { \Delta X } ^ { 0 } , \cdots , \widetilde { \Delta X } ^ { s } )$ is no greater than $2 ^ { - \mathcal { \ell } }$ , one would conclude the resistance of the block cipher against such differential cryptanalysis. 

## 2.3 Active S-box, Differential Distribution Table and Branch Number

We introduce some notions of active S-box, differential distribution table and branch number which will be used in our approach. 

Active S-boxes. The number of active S-boxes can be used to upper bound the MaxEDCP. Assume S-boxes are distinct in the cipher Enc. 

Definition 2.4. [Heys 2002b] Given a key ??, an input $X ^ { 0 }$ and an input difference $\Delta X ^ { 0 }$ to the cipher Enc, an S-box S is active if the two inputs to $s$ are distinct in the two executions of the cipher Enc under two inputs $( K , X ^ { 0 } )$ and $( K , { \bar { X } } ^ { 0 } \oplus \Delta X ^ { 0 } )$ , otherwise it is inactive. 

We denote by $N _ { \mathsf { d i f f } }$ the minimum number of the active S-boxes in all the possible pairs of executions. The probability of optimal ??-round differential characteristics is bounded from above by $p ^ { N _ { \mathrm { d i f f } } } \ \mathrm { [ H e y s 2 0 0 \bar { 2 } b ] }$ , where $\boldsymbol { p }$ denotes the maximum probability $\mathsf { P r } _ { S } ( \Delta X , \Delta Y )$ among all the nonzero differentials (Δ??, Δ?? ) for any S-box S which is active in the ??-round differential characteristics. 

Differential distribution table. A differential distribution table (DDT) is a data structure to represent the distribution $\mathsf { P r } _ { f }$ of a function ?? for all possible differentials. It also explicitly expresses the dependency (i.e., feasibility) between input and output differences of the function $f .$ . 

Definition 2.5. Given a function $f : \mathbb { B } ^ { n _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { B } ^ { n _ { i } } \to \mathbb { B } ^ { m _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { B } ^ { m _ { j } }$ , its DDT $\mathcal { D } _ { f }$ is table such that for every vector of input differences $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } )$ and every vector of output differences $( \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta \dot { Y } ^ { j } )$ , the entry $\mathcal { D } _ { f } ( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ gives the number of vectors of inputs $( X ^ { 1 } , \cdot \cdot \cdot , X ^ { i } ) \in \mathbb { B } ^ { n _ { 1 } } \times \cdot \cdot \cdot \times \bar { \mathbb { B } } ^ { n _ { i } }$ such that 

$$
f (X ^ {1}, \dots , X ^ {i}) \oplus f (X ^ {1} \oplus \Delta X ^ {1}, \dots , X ^ {i} \oplus \Delta X ^ {i}) = (\Delta Y ^ {1}, \dots , \Delta Y ^ {j}).
$$

The probability $\mathsf { P r } _ { f } ( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ can be deduced from the DDT $\mathcal { D } _ { f } \mathrm { : }$ : 

$$
\mathsf {P r} _ {f} (\Delta X ^ {1}, \dots , \Delta X ^ {i}, \Delta Y ^ {1}, \dots , \Delta Y ^ {j}) = \frac {\mathcal {D} _ {f} (\Delta X ^ {1} , \cdots , \Delta X ^ {i} , \Delta Y ^ {1} , \cdots , \Delta Y ^ {j})}{2 ^ {n _ {1} + \cdots + n _ {i}}}.
$$

We say the the vector of input and output differences $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ is feasible for $f ,$ if the probability $\mathsf { P r } _ { f } ( \bar { \Delta } X ^ { 1 } , \cdot \cdot \cdot , \Delta \bar { X } ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ is nonzero, otherwise it is infeasible. When the input space of the function ?? is small, its DDT $\mathcal { D } _ { f }$ can be computed by enumeration. 

Example 2.6. Consider the AND operation $\wedge : \mathbb { B } \times \mathbb { B } \to \mathbb { B }$ . We have that 

$$
\Delta Y = (X ^ {1} \wedge X ^ {2}) \oplus ((X ^ {1} \oplus \Delta X ^ {1}) \wedge (X ^ {2} \oplus \Delta X ^ {2})).
$$

The DDT $\mathcal { D } _ { \wedge }$ is shown in Table 1, e.g., $\begin{array} { r } { \mathsf { P r } _ { \wedge } ( 0 , 0 , 0 ) = \frac { 4 } { 4 } = 1 } \end{array}$ , and $\mathsf { P r } _ { \wedge } ( \Delta X ^ { 1 } , \Delta X ^ { 2 } , \Delta Y ) = \textstyle \frac { 2 } { 4 } = \textstyle \frac { 1 } { 2 }$ if Δ?? 1 = 1 or/and $\Delta X ^ { 2 } = 1$ for any fixed Δ?? . 

The DDT $\mathcal { D } _ { \vee }$ of the OR operation $\mathsf { V } : \mathbb { B } \times \mathbb { B } \to \mathbb { B }$ is the same as $\mathcal { D } _ { \wedge }$ . We can observe that the input difference (0, 0) cannot lead to the output difference 0 for both the AND and OR operations. □ 

Branch number. The (differential) branch number of a function is also used to characterize the dependency between input and output differences of the function [Daemen and Rijmen 1999]. 


Table 1. The DDT $( \mathcal { D } _ { \wedge } , \mathcal { D } _ { \vee } )$ for $\wedge / \vee : \mathbb { B } \times \mathbb { B }  \mathbb { B } .$ .


<table><tr><td><eq>(\Delta X^{1}, \Delta X^{2})</eq></td><td>(0,0)</td><td>(0,1)</td><td>(1,0)</td><td>(1,1)</td></tr><tr><td><eq>\Delta Y = 0</eq></td><td>4</td><td>2</td><td>2</td><td>2</td></tr><tr><td><eq>\Delta Y = 1</eq></td><td>0</td><td>2</td><td>2</td><td>2</td></tr></table>


Table 2. Branch numbers of $+ , - , \wedge , \vee , \oplus .$


<table><tr><td></td><td>+</td><td>-</td><td><eq>\wedge</eq></td><td><eq>\vee</eq></td><td><eq>\oplus</eq></td></tr><tr><td><eq>\mathcal{B}_{\mathrm{ww}}^{\min}</eq> and <eq>\mathcal{B}_{\mathrm{bw}}^{\min}</eq></td><td>2</td><td>2</td><td>1</td><td>1</td><td>2</td></tr><tr><td><eq>\mathcal{B}_{\mathrm{ww}}^{\max}</eq></td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td></tr><tr><td><eq>\mathcal{B}_{\mathrm{bw}}^{\max}</eq></td><td>3n-1</td><td>3n-1</td><td>3n</td><td>3n</td><td>2n</td></tr></table>

Definition 2.7. Given a function $f : \mathbb { B } ^ { n _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { B } ^ { n _ { i } } \to \mathbb { B } ^ { m _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { B } ^ { m _ { j } }$ , its minimum (resp. maximum) word-wise branch number $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m i n } } ( f )$ (resp. $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m a x } } ( f ) )$ is defined as 

$$
\mathcal {B} _ {\mathrm{ww}} ^ {\max} (f) = \max \left\{\operatorname{cnt} \left(\Delta X ^ {1}, \dots , \Delta X ^ {i}, \Delta Y ^ {1}, \dots , \Delta Y ^ {j}\right) \mid \forall 1 \leq \ell \leq n. X ^ {\ell}, \Delta X ^ {\ell} \in \mathbb {B} ^ {n _ {\ell}}. \text { BNCond } (f) \right\}
$$

$$
\mathcal {B} _ {\mathrm{ww}} ^ {\min} (f) = \min \left\{\operatorname{cnt} \left(\Delta X ^ {1}, \dots , \Delta X ^ {i}, \Delta Y ^ {1}, \dots , \Delta Y ^ {j}\right) \mid \forall 1 \leq \ell \leq n. X ^ {\ell}, \Delta X ^ {\ell} \in \mathbb {B} ^ {n _ {\ell}}. \text {BNCond} (f) \right\}
$$

where cnt $\cdot ( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ counts the number of nonzero entries in the vector $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ and the branch-number condition BNCond(?? ) is: 

$$
\mathsf {B N C o n d} (f) = \binom{(\Delta X ^ {1} \neq 0 \lor \dots \lor \Delta X ^ {i} \neq 0) \land (Y ^ {1}, \dots , Y ^ {j}) = f (X ^ {1}, \dots , X ^ {i})}{\land (Y ^ {1} \oplus \Delta Y ^ {1}, \dots , Y ^ {j} \oplus \Delta Y ^ {j}) = f (X ^ {1} \oplus \Delta X ^ {1}, \dots , X ^ {i} \oplus \Delta X ^ {i})}.
$$

Likewise, the minimum (resp. maximum) bit-wise branch number $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m i n } } ( f ) \left( \mathrm { r e s p . } \mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m a x } } ( f ) \right)$ of the function ?? is defined except that cnt $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ counts the number of 1 bits in the bitstream $\Delta X ^ { 1 } \Vert \cdot \cdot \cdot \Vert \bar { \Delta X ^ { i } } \Vert \Delta Y ^ { 1 } \Vert \cdot \cdot \cdot \Vert \Delta Y ^ { j }$ , i.e., Hamming weight. 

Intuitively, when the input differences of the function ?? are not all 0 bits (i.e., $\Delta X ^ { 1 } \neq 0 \lor \cdots \lor \Delta X ^ { i } \neq$ 0), the number of nonzero entries in any input and output differences $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ of the function ?? ranges from $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m i n } } ( f )$ to $\mathcal { B } _ { \sf w w } ^ { \mathrm { m a x } } ( f )$ , and the Hamming weight of any bitstream $\Delta X ^ { 1 } \| \cdot \cdot \cdot \| \Delta X ^ { i } \| \Delta Y ^ { 1 } \| \cdot \cdot \cdot \| \Delta Y ^ { j }$ ranges from $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m i n } } ( f )$ to $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m a x } } ( f )$ . 

Example 2.8. Consider the function $f _ { \oplus } : \mathbb { B } ^ { n } \times \mathbb { B } ^ { n } \to \mathbb { B } ^ { n }$ such that $f _ { \oplus } ( X ^ { 1 } , X ^ { 2 } ) = X ^ { 1 } \oplus X ^ { 2 }$ . We have: $\bar { \mathcal { B } } _ { \mathrm { w w } } ^ { \mathrm { m i n } } ( \bar { f } _ { \oplus } ) = \mathcal { B } _ { \mathrm { w w } } ^ { \mathrm { m a x } } ( f _ { \oplus } ) = \mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m i n } } ( f _ { \oplus } ) = 2$ and $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m a x } } ( f _ { \oplus } ) = 2 n$ . From $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m i n } } ( f _ { \oplus } ) = 2$ , we can deduce that at least two of $( X ^ { 1 } , X ^ { 2 } , X ^ { 1 } \oplus X ^ { 2 } )$ are nonzero if some of $( X ^ { 1 } , X ^ { 2 } )$ is nonzero. 

Similarly, $\mathcal { B } _ { \mathrm { w w } } ^ { \mathrm { m i n } } ( f _ { \odot } ) , \mathcal { B } _ { \mathrm { w w } } ^ { \mathrm { m a x } } ( f _ { \odot } ) , \mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m i n } } ( f _ { \odot } )$ and $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m a x } } ( f _ { \odot } )$ for each bit-wise operation $\odot \in \{ + , - , \land , \lor \}$ $\mathcal { B } _ { \mathsf { w w } , \odot } ^ { \mathrm { m i n } } , \mathcal { B } _ { \mathsf { w w } , \odot } ^ { \mathrm { m a x } } , \mathcal { B } _ { \mathsf { b w } , \odot } ^ { \mathrm { m i n } }$ $\mathcal { B } _ { \mathrm { b w , \odot } } ^ { \mathrm { m a x } }$ $\odot \in \{ + , - , \land , \lor , \oplus \}$ for a given matrix ?? and S-box S, $\mathcal { B } _ { \mathrm { w w } , M } ^ { \operatorname* { m i n } } , \mathcal { B } _ { \mathrm { w w } , M } ^ { \operatorname* { m a x } } , \mathcal { B } _ { \mathrm { b w } , M } ^ { \operatorname* { m i n } } , \mathcal { B } _ { \mathrm { b w } , M } ^ { \operatorname* { m a x } } , \mathcal { B } _ { \mathrm { w w } , S } ^ { \operatorname* { m i n } } , \mathcal { B } _ { \mathrm { w w } , S } ^ { \operatorname* { m a x } } , \mathcal { B } _ { \mathrm { b w } , S } ^ { \operatorname* { m i n } }$ ww,?? and $\mathcal { B } _ { \flat \ w , S } ^ { \mathrm { m a x } }$ are defined accordingly for the linear transformation $f _ { M } ( x ) = M * x$ and substitution $f _ { S } = S ( { \boldsymbol { x } } )$ . □ 

## 3 THE DESIGN OF EASYBC

EasyBC is a high-level, statically-typed, C-like cryptography-specific language, designed for conveniently describing block ciphers but without complicating the subsequent automated security analysis, so that cryptographers can quickly implement and analyze a block cipher especially during the design phase. 

## 3.1 Syntax

The syntax of EasyBC is given in Figure 1. 

Boxes. EasyBC features one standard array type and four cryptography-specific array types decorated by sbox, pbox, $\mathsf { p b o x } _ { \mathsf { m } }$ and ffm, respectively, that are commonly used for implementing block ciphers. The cryptography-specific arrays are global and immutable, thus called boxes in this paper. In particular, sbox defines an array that acts as a lookup-table based S-box for transforming ?? input bits to ?? output bits; pbox defines an array for performing permutation; $\mathsf { p b o x } _ { \mathfrak { m } }$ defines a matrix for linear transformation via matrix-vector product. (Note that pbox can be implemented via pb $) \mathsf { X } _ { \mathsf { m } }$ , we provide both for convenience.) ffm describes the finite-field multiplication (⊗) which may vary in block ciphers, thus should be defined by users. An ffm box is required only for performing linear transformations using $\mathsf { p b o x } _ { \mathsf { m } } ,$ , i.e., evaluating expressions of the form $M * x$ , thus cannot be explicitly involved in any statements. In this work, we assume a finite field of characteristic 2, so the finite-field addition is the bit-wise XOR (⊕). 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/538729ddca5af4fcdedc0ec1170883a080e7e708d90b4e2c39f41ffe72af1715.jpg)



Fig. 1. Syntax of EasyBC.


Positions. Position $\xi$ is used to express array indices for array get $( e [ \xi ] )$ , array slice (View $( e , \xi _ { 1 } , \xi _ { 2 } ) )$ , array put $\left( x [ \xi ] = e \right)$ and array left/right-rotation $( e \ll \xi \operatorname { a n d } e \gg \xi )$ via the common operations $\{ + , - , * , / , \% \}$ , whose values can be statically determined after preprocessing (i.e., independent of inputs). After preprocessing, all the positions $\xi$ will be constants. 

Expressions. Expressions are defined as usual, including modular addition (+), modular substitution (−), bit-wise AND $( \wedge )$ , bit-wise OR (∨), bit-wise NOT (∼) and bit-wise XOR (⊕), as well as common cryptography-specific operations. 

?? ∗ ?? is a matrix-vector product using finite-field multiplication (⊗) and addition (⊕), where the matrix ?? must be defined as an array of type $\mathsf { p b o x } _ { \mathfrak { m } }$ uint?? [??] [??] and the vector ?? should be an array of type uint?? [??]. $x \langle \cdot e \cdot \rangle$ is provided for performing permutation, where ?? is a P-box. Similarly, $x \langle e \rangle$ is provided for performing substitution, where ?? is an S-box. View $( e , \xi _ { 1 } , \xi _ { 2 } )$ is a slice of the array ?? starting at the index $\xi _ { 1 }$ and ending at the index $\xi _ { 2 }$ (inclusive), e.g., $\mathsf { V i e w } ( ( a , b , c , d ) , 1 , 3 )$ is $( b , c , d )$ . touint $( e _ { 0 } , \cdots , e _ { s - 1 } )$ transforms the ??-bitstream $( e _ { 0 } , \cdots , e _ { s - 1 } )$ into an ??-bit unsigned integer ?? such that bin $( x ) = ( e _ { 0 } , \cdot \cdot \cdot , e _ { s - 1 } ) , \mathrm { e . g . }$ ., touint(1, 1, 0, 1) is the 4-bit unsigned integer 13. touint(??) is the same as touint $( e [ 0 ] , \cdot \cdot \cdot , e [ n - 1 ] )$ for the array ?? of type uint1[??]. An array ?? can be left (resp. right) rotated $\xi$ positions via $e \ll \xi$ (resp. $e \gg \xi )$ , which can be seen special length-preserving permutations. $e [ \xi ]$ is array get, the same as View(??, ??, ??). 

Statements. Statements in EasyBC can be declarations, assignments, array puts, returns and round function calls. Note that EasyBC does not support branching statements (e.g., if-then-else), because current EasyBC suffices to implement both encryption and decryption processes of block ciphers while branching statements will make modeling complicated when analyzing security. We will evaluate the expressive capability of EasyBC in Section 8.1. 

Statement $\tau { \mathrm { ~ } } x = e$ is a syntactic sugar of $\tau x ; x = e$ . Statement for $( x$ from $n _ { 1 }$ to $n _ { 2 } ) \{ S \}$ is a range-for loop. Note that the range $[ n _ { 1 } , n _ { 2 } ]$ is limited to constants, which suffices to express block ciphers. The range variable ?? should be typed as uint, thus could be used in computing indices $\xi .$ 

Functions. EasyBC has three types of functions decorated by $\mathsf { r \_ f n } , \mathsf { s \_ s }$ fn or fn. A function decorated by $\boldsymbol { \mathsf { r } } _ { - } \mathsf { f } \boldsymbol { \mathsf { n } }$ is a round function, whose formal parameters are fixed to be the round number $r ,$ subkey ????, and text ?????? . Note that the subkey ???? and input text $t x t$ should have the same element type uint?? but may differ in number of elements, and the input text ?????? and output of a round function should have the same type uint?? [??]. An s_fn function is an alternative way to perform substitution instead of using arrays. Small S-boxes (e.g., 4-bit S-box in PRESENT [Bogdanov et al. 2007]) can be easily expressed as arrays, which can facilitate the follow-up security analysis. However, it would be infeasible to express large S-boxes as arrays (e.g., 64-bit S-box [Beierle et al. 2020]), for which s_fn functions can be used. An fn function defines the encryption (resp. decryption) process of a block cipher. Its parameters include the key ?? and plaintext (resp. ciphertext) ??????. The function body comprises declarations and round function calls for computing the ciphertext (resp. plaintext). 

Programs. A program ?? in EasyBC consists of a cipher name (decorated by @cipher), definitions of global boxes and functions, and a definition of an fn function describing the cipher. 

In this work, following [Mouha et al. 2011; Wu and Wang 2012; Zhang et al. 2018; Zhou et al. 2019], we consider single-key differential cryptanalysis, namely, the key is fixed and has no difference in any pair of executions, thus key schedule algorithms are omitted in EasyBC programs. Nevertheless, EasyBC can be easily extended to related-key differential cryptanalysis [Biryukov and Nikolić 2010] where the key may differ in some pairs of executions, by introducing a key schedule function. We leave this as interesting future work. 

Core language. The (full) language of EasyBC is designed for conveniently describing block ciphers with rich but redundant constructs. To ease the automation of the subsequent security analysis, we have identified a subset of EasyBC as the core language (cf. Figure 1), i.e., in the places highlighted in yellow, positions $\xi$ and expressions ?? are limited to constants and variables, respectively; in the places highlighted in grey constructs will not be present (i.e., they are to be eliminated by preprocessing the program written in the full language). A program in the core language is obtained by performing loop unrolling, constant-folding, constant propagation and dead-code elimination, on which type checking and security analysis are performed. 

Example 3.1. Figure 2 shows a snippet of the 64-bit block cipher PRESENT in EasyBC. The array s is an S-box for substitution, the array p is the P-box for permutation. The round function f1 is invoked during the 1-st to the 31-st round. Given the subkey sk and text t, f1(??,sk,t) for $1 \leq i \leq 3 1$ produces the output rtn of ??-th round. In detail, the input t is XORed with the subkey sk and results in the array nt, then the second range-for loop slices nt into 16 arrays via calling View, each of which is substituted via the S-box s, resulting in the array s_out. The array s_out is processed by applying the array p to perform permutation. Finally, the result rtn returned. 

Remarks on the design choice of EasyBC. In EasyBC, we introduce high-level constructs View, touint and permutations $( \mathrm { i } . \mathrm { e } . , x \langle \cdot e \cdot \rangle$ and $M * e )$ to ease the implementation of block ciphers. The inputs and outputs of round functions are fixed-size blocks which can be implemented by arrays (e.g., t and rtn in Figure 2). Typically, a block is to be split into small ones on which a look-up table based S-box (e.g., array in EasyBC) of suitable size is applied (e.g., 4-bit S-box s in Figure 2). The outputs of S-boxes will be juxtaposed to form a large block on which permutations are performed via either matrix-vector product or P-boxes $( \mathrm { e . g . , p 1 } \langle \mathrm { \cdot s \_ o u t \cdot } \rangle$ in Figure 2). On the other hand, to ease the automation of the subsequent security analysis, most indices are limited to positions $\xi$ whose values can be statically determined $( \mathrm { e . g . , } e [ \xi ]$ and View $( e , \xi _ { 1 } , \xi _ { 2 } ) )$ , and thus become constants after preprocessing. The loop-up table based S-boxes $( \mathrm { i } . \mathrm { e } . , x \langle e \rangle )$ are an exception as they require a complicated modeling method. It remains open how to handle generic array access with variable indices although currently there appears no such need to specify block ciphers. 

```lisp
@cipher PRESENT
sbox uint4[16] s = {12,5,6,11,9,0,10,13,3,14,15,8,4,7,1,2};
pbox uint[64] p = {0,16,32,48,1,17,33,...15,31,47,63};
r_fn uint1[64] f1(uint r, uint1[64] sk, uint1[64] t){
    uint1[64] nt;
    for(i from 0 to 63){ nt[i] = t[i]^sk[i]; }
    uint1[64] s_out;
    for(i from 0 to 15) {
    uint1[4] temp = View(nt, i*4, i*4+3);
    uint4 sbox_in = touint(temp[0], temp[1], temp[2], temp[3]);
    uint4 sbox_out = s1<sbox_in> # substitution via S-box
    s_out[i*4]=sbox_out[0]; s_out[i*4+1]=sbox_out[1];
    s_out[i*4+2]=sbox_out[2]; s_out[i*4+3]=sbox_out[3];
    }
    uint1[64] rtn = p1<·s_out>; # permutation via P-box
    return rtn; }
fn uint1[64] enc(uint1[2048] key, uint1[64] plaintext){
    uint1[64] text= plaintext;
    for(i from 1 to 31){ text=f1(i,View(key,(i-1)*64,i*64-1),text); }
    ... # execute the last rounds
    return text; } 
```


Fig. 2. Code snippet of the 64-bit block cipher PRESENT in EasyBC.


## 3.2 Operational Semantics

Let X denote a set of variables. An (evaluation) context $\sigma : \mathbb { X } \to \bigcup _ { i \geq 1 } \mathbb { N } ^ { i }$ is a mapping from variables to values, where a value can be a (fixed-width) non-negative integer or an array. Let $\sigma [ x \mapsto v ]$ be the context such that $\sigma [ x \mapsto v ] ( y ) = v { \mathrm { ~ i f ~ } } x = y ,$ otherwise $\sigma [ x \mapsto v ] ( y ) = \sigma ( y )$ . 

The evaluation judgement is in the form of 

$$
\sigma \models e: v
$$

meaning that the expression ?? evaluates to the value ?? under the evaluation context ??. 

The evaluation rules are given in Figure 3 (top-part), most of which are standard. Rule (S-Box) states that $x \langle y \rangle$ is a substitution, namely, the entry of the S-box ?? (??) at the index $\sigma ( y )$ . Rule (P-Box1) states that ?? ∗ ?? is the matrix-vector product of the matrix ?? and the array $\sigma ( x )$ . Rule (P-Box2) states that ?? ⟨·??·⟩ is a permutation of the array $\sigma ( y )$ according to the indices given by the P-box $\sigma ( x ) = \left( j _ { 0 } , \cdots , j _ { n - 1 } \right)$ , where its entry at the index ?? is the entry of the array $\sigma ( y )$ at the index $j _ { i \cdot }$ . 

The operational semantics of statements is defined as transition rules of the form 

$$
(\sigma , S) \Rightarrow \sigma^ {\prime}
$$

meaning that the execution of the statement ?? from the state ?? results in the state $\sigma ^ { \prime }$ . For a sequence of statements $S _ { 1 } ; S _ { 2 } ; \cdot \cdot \cdot S _ { n } ; , ( \sigma _ { 0 } , S _ { 1 } ; S _ { 2 } ; \cdot \cdot \cdot S _ { n } ; ) \Rightarrow ^ { + } \sigma _ { n }$ denotes the transitive transition of ⇒, i.e., $( \sigma _ { 0 } , S _ { 1 } ) \Rightarrow \sigma _ { 1 } , ( \sigma _ { 1 } , S _ { 2 } ) \Rightarrow \sigma _ { 2 } , \cdot \cdot \cdot , ( \sigma _ { n - 1 } , S _ { n } ) \Rightarrow \sigma _ { n }$ . The transition rules of EasyBC are listed in Figure 3 (bottom-part), which are standard. We denote by $\sigma _ { 0 } S _ { 1 } \sigma _ { 1 } S _ { 2 } \sigma _ { 2 } \cdot \cdot \cdot S _ { n } \sigma _ { n }$ the execution of the program ?? starting from the state $\sigma _ { 0 }$ and ending at the state $\sigma _ { n }$ , and $( \sigma _ { i - 1 } , S _ { i } ) \Rightarrow \sigma _ { i }$ for $1 \leq i \leq n$ . 

For an execution of an ??-IBC, we have 

(1) round functions can only be invoked in the fn function, 

(2) the first arguments in the invoked round functions are the round numbers $1 , 2 , 3 , \cdots , r ,$ and 

(3) the input and output of ??-th round are the third argument and return value of the invoked round function whose first argument is ??. 

$$
\begin{array}{l} \frac {}{\sigma \models n : n} (\text { Const }) \quad \frac {\sigma (x) = v}{\sigma \models x : v} (\text { Var }) \quad \frac {\sigma (x) = v \quad v ^ {\prime} = \sim v}{\sigma \models \sim x : v ^ {\prime}} (\text { Not }) \\ \frac {\sigma \left(x _ {1}\right) = n _ {1} \quad \sigma \left(x _ {2}\right) = n _ {2} \quad n = n _ {1} \odot n _ {2}}{\sigma \vDash x _ {1} \odot x _ {2} : n} (\mathrm{Op}) \quad \frac {\sigma (x) = \left(v _ {0} , \cdots , v _ {n - 1}\right) \quad \sigma (y) = i}{\sigma \vDash x \langle y \rangle : v _ {i}} (\text {S - Box}) \\ \frac {\sigma (x) = (v _ {0} , \cdots , v _ {n - 1}) , \quad \vec {v} = \big (\bigoplus_ {j = 0} ^ {n - 1} (M _ {0 , j} \otimes v _ {j}) , \cdots , \bigoplus_ {j = 0} ^ {n - 1} (M _ {n - 1 , j} \otimes v _ {j}) \big)}{\sigma \models M * x : \vec {v}} (P-Box_{1)} \\ \frac {\sigma (x) = \left(j _ {0} , \cdots , j _ {n - 1}\right) \quad \sigma (y) = \left(v _ {0} , \cdots , v _ {n - 1}\right) \quad \vec {v} = \left(v _ {j _ {0}} , \cdots , v _ {j _ {n - 1}}\right)}{\sigma \models x \langle y \cdot \rangle : \vec {v}} (P-Box_{2)} \\ \frac {\sigma (x) = \left(v _ {0} , \cdots , v _ {n - 1}\right) \quad \vec {v} = \left(v _ {i} , \cdots , v _ {j}\right)}{\sigma \models \operatorname{View} (x , i , j) : \vec {v}} (\text { VIEW }) \quad \frac {\operatorname{bin} (n) = \left(b _ {0} , \cdots , b _ {- 1}\right)}{\sigma \models \operatorname{touint} \left(b _ {0} , \cdots , b _ {- 1}\right) : n} (\text { Touint }) \\ \end{array}
$$

$$
\begin{array}{l} \frac {}{(\sigma , \tau x) \Rightarrow \sigma} (\text { DECL }) \quad \frac {\sigma \models e : v \quad \sigma^ {\prime} = \sigma [ x \mapsto v ]}{(\sigma , x = e) \Rightarrow \sigma^ {\prime}} (\text { Ass }) \\ \frac {\sigma (x) = \left(v _ {0} , \cdots , v _ {n - 1}\right) \quad \sigma (y) = v \quad \sigma^ {\prime} = \sigma \left[ x \mapsto \left(v _ {0} , \cdots , v _ {i - 1} , v , v _ {i + 1} , \dots , v _ {n - 1}\right) \right]}{\left(\sigma , x [ i ] = y\right) \Rightarrow \sigma^ {\prime}} (\text {ARR - PUT}) \\ \tau_ {0} f \left(\tau_ {1} x _ {1}, \dots , \tau_ {m} x _ {m}\right) \left\{S ^ {+} \text {return} y; \right\} v _ {1} = \sigma \left(y _ {1}\right), \dots , v _ {m} = \sigma \left(y _ {m}\right) \\ \frac {\sigma_ {\text {in}} = \sigma [ x _ {1} \mapsto v _ {1} ] \cdots [ x _ {m} \mapsto v _ {m} ] \quad (\sigma_ {\text {in}}, S ^ {+}) \Rightarrow^ {+} \sigma_ {\text {out}} \quad \sigma_ {\text {out}} (y) = v \quad \sigma_ {\text {ret}} = \sigma [ x \mapsto v ]}{(\sigma , x = f (y _ {1} , \cdots , y _ {m})) \Rightarrow \sigma_ {\text {in}} \quad (\sigma_ {\text {out}} , \text {return} y) \Rightarrow \sigma_ {\text {ret}}} \tag {CALL-RET} \\ \end{array}
$$

Fig. 3. The operational semantics of core EasyBC, where $\odot \in \{ + , - , \oplus , \wedge , \vee \}$ 

## 3.3 Type System of Core EasyBC

The type system of core EasyBC is designed to disallow certain kinds of illegal programs and provide type information for security analysis. 

EasyBC supports the following types, i.e., 

$$
\beta : := \text {   uints   } | \text {   uint   } | \text {   uints   } [ n ] | \text {   sbox   uints   } [ n ] | \text {   pbox   uint   } [ n ] | \text {   pbox } _ {\mathfrak {m}} \text {   uints   } [ n ] [ n ].
$$

Here, uint?? is for ??-bit unsigned integers, uint?? [??] is for vectors (or arrays) of ??-bit unsigned integers, uint is for unsigned integers, and uint[??] is for vectors (or arrays) of unsigned integers. Note that uint?? is identical to uint1[??] and uint?? [1]. uint and uint[??] are used only when the variables under typing are independent of inputs. 

Typing expressions. The typing judgement is of the form of 

$$
T _ {g}, T _ {l} \vdash e: \beta ,
$$

where $( T _ { g } , T _ { l } )$ is a typing context, ?? is an expression under typing, and $\beta$ is a type. The global environment $T _ { g }$ is a mapping from global variables to their types and from function names to function signatures $( \beta _ { 0 } , \cdots , \beta _ { n } )$ where $\beta _ { 0 }$ is the return type and $\beta _ { 1 } , \cdots , \beta _ { n }$ are the types of the formal parameters. The local environment $T _ { l }$ is a mapping from local variables to their types. The typing judgement $T _ { q } , T _ { l } \vdash e : \beta$ is valid if ?? has type $\beta$ under the typing context $( T _ { q } , T _ { l } )$ . 

Figure 4 (top-part) gives typing rules for expressions. Rule (T-Uints) express that a non-negative integer ?? can be typed as uint?? if $n \leq 2 ^ { s } - 1$ . Rules (T-Var) and (T-Not) are defined as usual. Rule (T-Op) ensures that the two operands and result of the operation $\odot \in \{ + , - , \oplus , \wedge , \vee \}$ have the same type uint??. Rule $\left( \mathbf { T } \mathbf { - } \mathbf { P B } \mathbf { O } \mathbf { X } _ { 1 } \right)$ ensures that the array ?? has suitable type w.r.t. the type of the matrix ?? for matrix-vector product. Rule $\left( \mathrm { { T } - \mathrm { { P B O X } _ { 2 } } } \right)$ requires that the elements in the array $x \langle \cdot y \cdot \rangle$ and the operand ?? have the same type, as the P-box ?? only specifies the element order for the permutation. Rule (T-Sbox) requires that the S-box has a sufficient number of elements $( \mathrm { i } . \mathrm { e } . , n \ge 2 ^ { s _ { 2 } } )$ and the 

$$
\begin{array}{l} \frac {0 \leq n \leq 2 ^ {s} - 1}{T _ {g} , T _ {l} \vdash n : \mathsf {u i n t s}} (\mathrm{T-UINTs}) \\ \frac {\odot \in \{+ , -, \oplus , \wedge , \vee \} \quad T _ {g} , T _ {l} \vdash x _ {i} : \texttt {u i n t s f o r} i = 1 , 2}{T _ {g} , T _ {l} \vdash x _ {1} \odot x _ {2} : \texttt {u i n t s}} (\text { T - Op }) \\ \frac {T = (x \in \mathbb {X} _ {f} ? T _ {l} : T _ {g})}{T _ {g} , T _ {l} \vdash x : T (x)} (\mathrm{T-V} _ {\mathrm{AR}}) \\ \frac {T _ {g} (M) = \mathtt {p b o x} _ {\mathfrak {m}}   \mathtt {u i n t s} [ n ] [ n ] \qquad T _ {g} , T _ {l} \vdash x : \mathtt {u i n t s} [ n ]}{T _ {g} , T _ {l} \vdash M * x : \mathtt {u i n t s} [ n ]}   (\mathrm{T-P} _ {\mathrm{BOX} _ {1}}) \\ \frac {T _ {g} , T _ {l} \vdash e : \tau}{T _ {g} , T _ {l} \vdash \sim e : \tau} (\mathrm{T-Not}) \\ \frac {T _ {g} (x) = \mathtt {p b o x u i n t} [ n _ {1} ] \qquad T _ {g} , T _ {l} \vdash y : \mathtt {u i n t s} [ n _ {2} ]}{T _ {g} , T _ {l} \vdash x \langle y \cdot \rangle : \mathtt {u i n t s} [ n _ {1} ]} (\mathrm{T-P} _ {\mathrm{BOX} _ {2}}) \\ \frac {T _ {g} (x) = \texttt {s b o x u i n t s} _ {1} [ n ] \qquad T _ {g} , T _ {l} \vdash y : \texttt {u i n t s} _ {2} \qquad n \geq 2 ^ {s _ {2}}}{T _ {g} , T _ {l} \vdash x \langle y \rangle : \texttt {u i n t s} _ {1}} (\texttt {T - S _ {B O X}}) \\ \frac {T _ {g} , T _ {l} \vdash x : \mathsf {u i n t s} [ n ] \quad 0 \leq n _ {1} \leq n _ {2} <   n \quad n ^ {\prime} = n _ {2} - n _ {1} + 1}{T _ {g} , T _ {l} \vdash \mathsf {V i e w} (x , n _ {1} , n _ {2}) : \mathsf {u i n t s} [ n ^ {\prime} ]} (\text {T - VIEW}) \\ \frac {T _ {g} , T _ {l} \vdash x _ {i} : \text { uint1   for } 0 \leq i <   s}{T _ {g} , T _ {l} \vdash \text { tuint } (x _ {0} , . . . , x _ {s - 1}) : \text { uints }} \tag {T-TOUINT} \\ \end{array}
$$

$$
\frac {T _ {l} (x) = \tau}{T _ {g} , T _ {l} , f \vdash \tau x} (\mathrm{T-DeCL})
$$

$$
\frac {T _ {l} (x) = \mathsf {u i n t s} [ n ] \quad T _ {g} , T _ {l} \vdash y : \mathsf {u i n t s} \quad 0 \leq i <   n}{T _ {g} , T _ {l} , f \vdash x [ i ] = y} (\text {T - ARR - PUT})
$$

$$
T _ {l} (x) = \tau
$$

$$
\frac {T _ {g} , T _ {l} \vdash e : \tau}{T _ {g} , T _ {l} , f \vdash x = e} (\text {T - Ass})
$$

$$
T _ {g} (f ^ {\prime}) = (\tau_ {0}, \dots , \tau_ {m})
$$

$$
\frac {T _ {g} , T _ {l} \vdash x : \tau_ {0} \quad T _ {g} , T _ {l} \vdash x _ {i} : \tau_ {i} \text {for} 1 \leq i \leq m}{T _ {g} , T _ {l} , f \vdash x = f ^ {\prime} (x _ {1} , \cdots , x _ {m})} (\mathrm{T-CALL})
$$

$$
\ell \in \{\text { fn }, r _ {-} \text { fn }, s _ {-} \text { fn } \} \quad T _ {g} (f) = (\tau_ {0}, \dots , \tau_ {m})
$$

$$
\frac {T _ {g} , T _ {l} [ p _ {1} \mapsto \tau_ {1} , \cdots , p _ {m} \mapsto t _ {m} , y \mapsto \tau_ {0} ] , f \vdash S _ {i} \text {for} 1 \leq i \leq n}{T _ {g} , T _ {l} \vdash \ell \tau_ {0} f (\tau_ {1} p _ {1} , \cdots , \tau_ {m} p _ {m}) \{S _ {1} ; \cdots ; S _ {n} ; \text {return} y ; \}} (\mathrm{T-FN-DEF})
$$

Fig. 4. The typing rules of core EasyBC. 

result $x \langle y \rangle$ has the same type as the elements in the S-box ??. Note that both S-boxes and P-boxes do not necessarily preserve the length which may occur, e.g., DES. Rule (T-View) requires that the indices $n _ { 1 }$ and $n _ { 2 }$ are within the bounds of the array ??, moreover, the slice $\mathsf { V i e w } ( x , n _ { 1 } , n _ { 2 } )$ is an array with length $n _ { 2 } - n _ { 1 } + 1$ and its elements have the same type as the elements in the array ??. Rule (T-Toint) requires that all the operands $x _ { i }$ have type uint1 and the result has type uint?? where ?? is the number of operands. 

Typing statements. The typing judgement of a statement is in the form of 

$$
T _ {g}, T _ {l}, f \vdash S
$$

where $( T _ { g } , T _ { l } )$ is a typing context, ?? is the statement under typing in the function $f .$ We write $T _ { g } , T _ { l } , f \vdash S$ is valid if ?? is well-typed. 

The typing rules are given in Figure 4 (middle-part). Rule (T-Decl) is defined as usual. Rule (T-Ass) requires that the type of the expression ?? conforms to the declared type of the variable ??. Rule (T-Arr-Put) requires that the index ?? is within the bounds of the array ?? and the operand ?? has the same type as the elements in the array ??. Rule (T-Call) requires that the types of actual arguments and return conform to the corresponding function signature $T _ { g } ( f ^ { \prime } )$ . 

Typing programs. Each program ?? is typed by iteratively typing each function definition. The program is well-typed if all the function definitions are well-typed. The typing judgement of a function definition fn_def is in the form of 

$$
T _ {g}, T _ {l} \vdash \mathsf {f n \_ d e f},
$$

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/5d1a7f1c55cf70bf71e04d629d7b6dfa0d85e5c69c67e09c5dd88eee21035dc0.jpg)



Fig. 5. Overview of our approach.


where $( T _ { g } , T _ { l } )$ is a typing context and fn_def is a function definition under typing. The typing judgement $T _ { g } , T _ { l } \vdash \mathsf { f n } .$ _def is valid if the function definition fn_def is well-typed. The typing rule (T-Fn-Def) is given in Figure 4 (bottom-part), which enforces the well-typed function body when the formal parameters and return have declared types. 

## 3.4 Compilation

We have implemented an interpreter in C++ for EasyBC to test the operational semantics of programs, making sure that they are consistent with the execution of reference block ciphers. In particular, we compare the output of EasyBC programs with that of running the binary executable compiled from C/C++ programs by GNU C++ compiler (G++). For each cryptographic primitive, we randomly generate inputs and then run the EasyBC program (with our interpreter) and the binary executable. We record their output, as well as the execution time for analysis. The results are given in Section 8.1. 

## 3.5 Overview of Analysis

Recall that we are interested in evaluating the resistance of block ciphers against differential cryptanalysis by bounding the MaxEDCP. A block cipher is considered to be resistant to differential cryptanalysis if MaxEDCP is no greater than $O ( 2 ^ { \ell } )$ for the block size $\mathcal { \ell } .$ 

Figure 5 gives an overview of our approach. Given a program in full EasyBC together with an option for selecting a particular MILP generation approach and an S-box modeling technique, EasyBC computes an upper bound of the MaxEDCP. The result is conclusive if this upper bound is sufficient to show the resistance of the program. Our approach is not necessarily complete (e.g., in most cases we only compute an upper bound of MaxEDCP), so it may fail to prove the resistance of some programs, although this does not happen in our evaluation (cf. Section 8). 

First, the input program is preprocessed to eliminate range-for loops and positional variables by performing loop unrolling, constant-folding, constant propagation and dead-code elimination. The final program will be in the core language of EasyBC. Hereafter, we assume that the given EasyBC program has been preprocessed. 

Next, the program is type-checked to disallow certain kinds of illegal programs, e.g., the types of operands in expressions, formal parameters in function definitions and actual arguments in function calls are proper. It also provides type information for security analysis, in particular, the lengths of arrays, the type and the bit widths of array elements, which are used for MILP generation. 

After type-checking, we reduce the problem of bounding the MaxEDCP to MILP. The key insight of the reduction is to characterize the dependency (i.e., feasibility) between input and output differences of each operation using integer linear (IL) constraints and bound the MaxEDCP by minimizing an objective function subject to the IL constraints. By utilizing an MILP solver (e.g., Gurobi [Gurobi Optimization 2018]), we can obtain an upper bound of the MaxEDCP. In practice, one may be only interested in proving the resistance against differential cryptanalysis. Hence we also verify whether the MaxEDCP is no greater than a given threshold, an affirmative answer to which would be sufficient to show that the given cipher is resistant against differential cryptanalysis. This strategy is very effective in practice, as few rounds are often sufficient to prove the resistance by leveraging the decomposition approach (cf. Proposition 5.3 and Proposition 7.3). 

To this end, we present two different approaches for reducing to MILP. The first one is by determining the lower bound of the minimum number $N _ { \mathrm { d i f f } }$ of active S-boxes in either wordwise (Section 5) or bit-wise (Section 6) manner, because the MaxEDCP of ??-round differential characteristics is bounded from above by $p ^ { N _ { \mathrm { d i f f } } }$ [Heys 2002b; Sun et al. 2014a], where $\boldsymbol { p }$ denotes the maximum probability $\mathsf { P r } _ { S } ( \Delta X , \Delta Y )$ among all the nonzero differentials (Δ??, Δ?? ) for any active S-box. Intuitively, the word-wise one models the difference of an ??-bitstream under two executions by only one Boolean variable, thus is less involved and produces fewer constraints, but is limited to certain block ciphers $\mathrm { e . g . }$ , it cannot be directly applied to bit-oriented block ciphers such as PRESENT). In contrast, the bit-wise approach models the difference of each bit by one Boolean variable, thus is more fine-grained and has wider applicability. One may understand that the bitwise approach implicitly bit-blasts the program and then generates MILP similar to the word-wise approach. The MILP generation in this approach requires the (maximum/minimum word-/bit-wise) branch numbers of some operations and representing S-boxes as IL constraints, for which we propose novel SMT-based methods to automatically determine branch numbers for each operation and implement some recent promising bit-wise S-box modeling techniques. 

The first approach is efficient and often effective, but the obtained upper bound may not be sufficiently tight and is not applicable for some ciphers. We provide an extended bit-wise approach to directly bound the MaxEDCP (Section 7). In the extended bit-wise approach, the probabilities between input and output differences for each operation are further encoded into IL constraints using additional Boolean variables. This approach may be less efficient but is more accurate than the first approach. We also propose a novel Maximal Satisfiability Modulo Theories (MaxSMT) [Bjørner and Phan 2014] based extended bit-wise S-box modeling method which guarantees that the least number of Boolean variables is used for encoding differential probabilities of S-boxes. 

## 4 UTILITIES

In this section, we present the three key utilities used in our MILP generation. 

## 4.1 SMT-based Method for Determining Branch Numbers

The branch numbers of some operations are required in MILP generation. However, to our best knowledge, existing work usually relies on manual analysis. In this paper, we propose to determine branch numbers of a given operation/function, by reducing to the optimization problem modulo bit-vector theory, which can be solved by off-the-shelf optimizing SMT solver, e.g., Z3 [Bjørner et al. 2015]. 

Given a function $f : \mathbb { B } ^ { n _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { B } ^ { n _ { i } } \to \mathbb { B } ^ { m _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { B } ^ { m _ { j } }$ , to compute its minimum (resp. maximum) word-wise branch number $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m i n } } ( f )$ (resp. $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m a x } } ( f ) )$ , by Definition 2.7, we express the condition BNCond(?? ) and the additional condition $d = \mathsf { c n t } ( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ as a quantifier-free SMT formula $\phi _ { f }$ in the bit-vector theory, and use Z3 to minimize (resp. maximize) the variable ?? subject to the SMT formula $\phi _ { f }$ . The optimized value of ?? is $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m i n } } ( f )$ (resp. $\mathcal { B } _ { \mathsf { w w } } ^ { \mathrm { m a x } } ( f ) )$ . An illustrating example is given in [Sun et al. 2023, Section C.1]. 

The minimum (resp. maximum) bit-wise branch number $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m i n } } ( f )$ (resp. $\mathcal { B } _ { \mathrm { b } w } ^ { \mathrm { m a x } } ( f ) )$ ) can be computed the same as above, except that cnt $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ counts the number of 1 bits in the bitstream $\Delta X ^ { 1 } \| \cdot \cdot \cdot \| \mathbf { \bar { \Delta } } \Delta X ^ { i } \| \Delta Y ^ { 1 } \| \cdot \cdot \cdot \| \Delta Y ^ { j }$ . 

Proposition 4.1. The minimized (maximized) value of ?? is 

• $\mathcal { B } _ { \mathrm { w w } } ^ { \mathrm { m i n } } ( f ) \left( r e s p . \mathcal { B } _ { \mathrm { w w } } ^ { \mathrm { m a x } } ( f ) \right)$ when cnt $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ counts the number of nonzero entries in the vector $( \Delta X ^ { 1 } , \cdot \cdot \cdot , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdot \cdot \cdot , \Delta Y ^ { j } )$ , 

• $\mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m i n } } ( f ) \ ( r e s p . \ \mathcal { B } _ { \mathrm { b w } } ^ { \mathrm { m a x } } ( f ) ) \ w h e n \ \mathrm { c n t } ( \Delta X ^ { 1 } , \cdots , \Delta X ^ { i } , \Delta Y ^ { 1 } , \cdots , \Delta Y ^ { j } )$ counts the number of 1 bits in the bitstream $\widetilde { \Delta { X } ^ { 1 } } \Vert \cdot \cdot \cdot \Vert \Delta { X } ^ { i } \Vert \Delta { Y } ^ { 1 } \Vert \cdot \cdot \cdot \Vert \Delta { Y } ^ { j }$ . 

## 4.2 Bit-wise S-box Modeling

Consider an expression $s \langle X \rangle$ where $S : \mathbb { B } ^ { n }  \mathbb { B } ^ { m }$ is a lookup-table based S-box. It is non-trivial to specify the bit-level dependency of differences between the input ?? and the result of $s \langle X \rangle$ (denoted by ?? ), as the S-box provides only input and output pairs. To resolve this issue, we first compute its DDT $\mathcal { D } _ { S } : \mathbb { B } ^ { n } \times \mathbb { B } ^ { m }  \mathbb { N }$ from which IL constraints are generated to characterize the bit-level dependency of differences between ?? and ?? . Recall that for every differential $( \Delta X , \Delta Y ) \in \mathbb { B } ^ { n } \times \mathbb { B } ^ { m }$ , $\mathcal { D } s ( \Delta { X } , \Delta { Y } )$ gives the number of inputs $X \in \mathbb { B } ^ { n }$ such that $S ( X ) \oplus S ( X \oplus \Delta X ) = \Delta Y$ . 

Let $\vec { b }$ be the input difference Δ?? and $\vec { b ^ { \prime } }$ be the output difference Δ?? of the S-box S. We have 

$\begin{array} { r } { \sum _ { i = 0 } ^ { n - 1 } \vec { b } _ { i } = 0 \Longrightarrow \sum _ { i = 0 } ^ { m - 1 } \vec { b } _ { i } ^ { \prime } = 0 , \mathrm { i . e } } \end{array}$ to $s$ are the same, then the two outputs are the same. This condition can be characterized by the IL constraint $\begin{array} { r } { m \cdot \sum _ { i = 0 } ^ { n - 1 } \vec { b } _ { i } \geq \sum _ { i = 0 } ^ { m - 1 } \vec { b } _ { i } ^ { \prime } } \end{array}$ , denoted by $\Psi _ { S } ^ { 1 }$ . 

• ${ \textstyle \sum _ { i = 0 } ^ { m - 1 } \vec { b } _ { i } ^ { \prime } = 0 \Rightarrow \textstyle \sum _ { i = 0 } ^ { n - 1 } \vec { b } _ { i } = 0 \mathrm { ~ i f ~ } }$ S is injective, i.e., no input difference if no output difference, namely, if two outputs of S are the same, then the two inputs must be the same. This condition can be exactly characterized by the IL constraint $\begin{array} { r } { n \cdot \sum _ { i = 0 } ^ { m - 1 } \vec { b } _ { i } ^ { \prime } \geq \sum _ { i = 0 } ^ { n - 1 } \vec { b } _ { i } } \end{array}$ , denoted by $\Psi _ { S } ^ { 2 }$ . 

• $\mathcal { D } _ { S } ( \vec { b } , \vec { b } ^ { \prime } ) \neq 0 ;$ , namely, $( \vec { b } , \vec { b } ^ { \prime } )$ should be feasible for S. We implement and compare the promising techniques [Abdelkhalek et al. 2017; Boura and Coggia 2020; Li and Sun 2022; Sasaki and Todo 2017; Sun et al. 2014b; Udovenko 2021] that can characterize $\mathcal { D } s ( \vec { b } , \vec { b } ^ { \prime } ) \neq 0$ by IL constraints. Hereafter, we denote by $\Psi _ { S } ^ { 3 }$ the set of IL constraints such that $( \vec { b } , \vec { b } ^ { \prime } )$ is a solution of $\Psi _ { S } ^ { 3 }$ iff $\mathcal { D } _ { S } ( \vec { b } , \vec { b } ^ { \prime } ) \neq 0$ . 

Proposition 4.2. $( \vec { b } , \vec { b } ^ { \prime } )$ is feasible input and output differences of S $i f ( \vec { b } , \vec { b } ^ { \prime } )$ is a solution $o f \Psi _ { S } ^ { 3 }$ 

We denote by $\Psi _ { S }$ the set $\Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 2 } \cup \Psi _ { S } ^ { 3 }$ if the S-box S is injective, otherwise $\Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 3 }$ 

## 4.3 MaxSMT-based Extended Bit-wise S-box Modeling

The above bit-wise S-box modeling method is able to characterize all the feasible differentials $( \Delta X , \Delta Y ) \in \mathbb { B } ^ { n } \times \mathbb { B } ^ { m }$ of the S-box S, but the probability $\begin{array} { r } { \mathsf { P r } _ { S } ( \Delta X , \Delta Y ) = \frac { \mathcal { D } _ { S } ( \Delta X , \Delta Y ) } { \mathcal { n } } } \end{array}$ of differentials is not present in the IL constraints, so it is impossible to bound the MaxEDCP directly. We propose a novel MaxSMT-based method which guarantees that the least number of Boolean variables is used for encoding probabilities of differentials. 

Definition 4.3. Given two sets of constraints $( \Phi _ { 1 } , \Phi _ { 2 } )$ , the MaxSMT problem is to find a solution that satisfies all the constraints in $\Phi _ { 1 }$ and maximizes the number of satisfied constraints in $\Phi _ { 2 }$ . 

Let $V = \{ v _ { 1 } , \cdots , v _ { h } \}$ be the set of nonzero probabilities $\mathsf { P r } _ { S } ( \Delta X , \Delta Y )$ for $( \Delta X , \Delta Y ) \in \mathbb { B } ^ { n } \times \mathbb { B } ^ { m }$ . We define the MaxSMT problem $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ for the S-box S, where 

$$
\Phi_ {1} ^ {\mathcal {S}} = \{\sum_ {i = 1} ^ {h} c _ {i} \cdot p _ {i, j} = - \log_ {2} v _ {j} \mid 1 \leq j \leq h \} \mathrm{and} \Phi_ {2} ^ {\mathcal {S}} = \{c _ {1} = 0, \dots , c _ {h} = 0 \},
$$

for $1 \leq i , j \leq h , c _ { i }$ is a variable over real numbers and $\mathbf { \nabla } \mathcal { P } \mathfrak { i } , \mathfrak { j }$ is a Boolean variable. Clearly, for every $1 \leq j \leq h , 2 ^ { - \sum _ { i = 1 } ^ { h } c _ { i } \cdot p _ { i , j } }$ retains the probability $v _ { j }$ . Since the Boolean variable $\mathit { p } _ { i , j }$ can be treated as a variable over real numbers by adding $p _ { i , j } = 0 \vee p _ { i , j } = 1$ to $\Phi _ { 1 } ^ { S }$ , and $v _ { j }$ (hence $\log _ { 2 } { v _ { j } } )$ is a constant for each $1 \leq j \leq h$ , the problem $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ is a MaxSMT problem (modulo the theory of real numbers). 

A solution of the MaxSMT problem $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ assigns values to the variables $c _ { i } { } ^ { \prime } { s }$ and $\mathop { p _ { i , j } ^ { \prime } s }$ from which the probability $v _ { j }$ for every $1 \leq j \leq h$ can be obtained. Suppose the solution assigns the values $\{ b _ { 1 , j } , \cdots , b _ { h , j } \}$ to the Boolean variables $\{ p _ { 1 , j } , \cdot \cdot \cdot , p _ { h , j } \}$ and the values $\{ t _ { 1 } , \cdots , t _ { h } \}$ to the variables $\{ c _ { 1 } , \cdots , c _ { h } \}$ , we have: $2 ^ { - \sum _ { i = 1 } ^ { h } t _ { i } \cdot b _ { i , j } } = v _ { j }$ . Note that $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ is always satisfiable. 

We can observe that if $t _ { i } = 0$ , the value $b _ { i , j }$ of the Boolean variable $\mathit { p } _ { i , j }$ for $1 \leq j \leq h$ can be omitted for retaining all the probabilities in ?? . We will see later that the values $\{ b _ { 1 , j } , \cdots , b _ { h , j } \}$ of the Boolean variables $\mathcal { P } i , j ^ { ^ { \ast } }$ will be used to encode the probabilities in ?? , we define $\Phi _ { 2 } ^ { S }$ as $\{ c _ { 1 } = 0 , \cdot \cdot \cdot , c _ { h } = 0 \}$ so that a solution of the MaxSMT problem $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ maximizes the number of 0 bits in the values $\{ t _ { 1 } , \cdots , t _ { h } \}$ of the variables $\{ c _ { 1 } , \cdots , c _ { h } \}$ , thus minimizing the number of additional Boolean variables used for encoding the probabilities in ?? . 

Let $\{ i _ { 1 } , \cdots , i _ { k } \}$ be the set of indices of the nonzero values in $\{ t _ { 1 } , \cdots , t _ { h } \}$ . To encode all the probabilities in ?? , we define an extended DDT $\mathcal { D } _ { s } ^ { \dagger }$ of the S-box S as follows: 

$$
\forall (\Delta X, \Delta Y) \in \mathbb {B} ^ {n} \times \mathbb {B} ^ {m}. 1 \leq j \leq h. \mathcal {D} _ {\mathcal {S}} ^ {\dagger} (\Delta X, \Delta Y, b _ {i _ {1}, j}, \dots , b _ {i _ {k}, j}) \neq 0 \text {   iff   } \operatorname * {P r} _ {\mathcal {S}} (\Delta X, \Delta Y) = v _ {j}.
$$

A set $\Psi _ { S } ^ { 4 }$ of constraints over the Boolean variables $\vec { b } , \vec { b ^ { \prime } } , p _ { i _ { 1 } } , \cdots , p _ { i _ { k } }$ can be generated from the extended DDT $\mathcal { D } _ { s } ^ { \dagger }$ (cf. Section 4.2) such that 

$$
\mathcal {D} _ {\mathcal {S}} ^ {\dagger} \left(\Delta X, \Delta Y, b _ {i _ {1}}, \dots , b _ {i _ {k}}\right) \neq 0 \text {   iff   } \left(\Delta X, \Delta Y, b _ {i _ {1}}, \dots , b _ {i _ {k}}\right) \text {   is   a   solution   of   } \Psi_ {\mathcal {S}} ^ {4}.
$$

Proposition 4.4. For any solution $\left( \Delta X , \Delta Y , b _ { i _ { 1 } } , \cdot \cdot \cdot , b _ { i _ { k } } \right) o f \Psi _ { S } ^ { 4 } , \mathsf { P r } _ { S } ( \Delta X , \Delta Y ) = 2 ^ { - \sum _ { j = 1 } ^ { k } t _ { i _ { j } } \cdot b _ { i _ { j } } } .$ 

We denote by $\Psi _ { S } ^ { \dagger }$ the set $\Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 2 } \cup \Psi _ { S } ^ { 4 }$ if the S-box S is injective, otherwise $\Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 4 }$ . An illustrating example is given in [Sun et al. 2023, Section C.2]. 

## 5 WORD-WISE APPROACH

In this section, we present an approach for determining the lower bound of the minimum number of active S-boxes by reducing to MILP in a word-wise fashion. It works for programs ?? where types are uint?? [??] for a fixed bit size ?? of the involved S-boxes and individual bits of any entry in an array cannot be changed. Our tool can automatically check if the word-wise approach is applicable. Recall that uint??, uint1[??] and uint?? [1] are identical, and we shall use uint?? [1] hereafter. Note that, in this setting, the program ?? is free of touint expressions. 

High-level intuition. Each entry of an array variable ?? in the program ?? is modeled as a Boolean variable ??, where $b = 1$ in the MILP solution indicates that the entry has a difference when $P$ is executed under two distinct inputs for some fixed key. Thus, a variable of type uint?? [??] is modeled by a vector $\vec { b }$ of ?? Boolean variables. Each statement is modeled by a set of IL constraints over the Boolean variables which characterize the propagation of differences through the statement. Furthermore, each S-box S is associated with a unique Boolean variable $b _ { S }$ such that the S-box S is active under two execution if $b _ { S } = 1$ (cf. Definition 2.4). The objective function is to minimize the sum of Boolean variables $b _ { S }$ for all the S-boxes $s ,$ , subject to the extracted IL constraints. The MILP solution gives the lower bound of the minimum number of active S-boxes in the program. 

The word-wise MILP generation rules are given by a word-wise differential denotational semantics of EasyBC. We present the denotational semantics for expressions in Section 5.1 and the denotational semantics for statements in Section 5.2. 

## 5.1 Word-wise Differential Denotational Semantics for Expressions

W denotes by $\mathbb { X } _ { f }$ the set of its local variables of a function $f ,$ and by $\mathbb { K } _ { f } \subseteq \mathbb { X } _ { f }$ the set of variables that are subkeys. We denote by $| x | = n \operatorname { i f } x$ has type uint?? [??]. 

<table><tr><td>[[View(x,i,j)]WY = (∅, (b̂i,···, b̂j)), where b̂ = γ(x)</td><td>[[~x]]WY = (∅,γ(x))</td></tr><tr><td colspan="2">[[x1+x2]]WY = [[x1-x2]]WY = [[x1 ⊕ x2]]WY = (Ψi2,3(b0,b1,b2), b0), where i=1,2, b0=newBV() 
Ψ12,3(b0,b1,b2) = {b1+b2≥b0,b0+b1≥b2, b0+b2≥b1} [Li et al. 2019], b1=γ(x1), b2=γ(x2) 
Ψ2,3(b0,b1,b2) = {b&#x27; ≥ b0,b&#x27; ≥ b1,b&#x27; ≥ b2, Σi=02bi ≥ 2b&#x27;} [Mouha et al. 2011], b&#x27; = newBV()</td></tr><tr><td colspan="2">[[x1 ∧ x2]]WY = [[x1 ∨ x2]]WY = ({{b1+b2≥b0},b0}), where b0 = newBV(), b1=γ(x1), b2=γ(x2)</td></tr><tr><td colspan="2">[[M * x]]WY = (ΨiM(b, b&#x27;), b&#x27;), where i=1,2, b̂ = γ(x), b̂ = newBV(), b&#x27;&#x27; = newBV() 
Ψ1M(b, b&#x27;) = {Bminww,M · b&#x27;&#x27; ≤ |x|-1(i+ b&#x27;i) ≤ Bmaxww,M, 2|x|·b&#x27;&#x27; ≥ |x|-1(i+ b&#x27;i)} 
Ψ2M(b, b&#x27;) = {Bminww,M · b&#x27;&#x27; ≤ |x|-1(i+ b&#x27;i) ≤ Bmaxww,M, b&#x27;&#x27; ≥ b0, b&#x27;&#x27; ≥ b0&#x27;,···, b&#x27;&#x27; ≥ b|x|-1, b&#x27;&#x27; ≥ b&#x27;|x|-1}</td></tr><tr><td colspan="2">[[x⟨y⟩]]WY = (∅, (b̂j0,···, b̂jn-1)), where x is P-box (j0,···, jn-1) and b̂ = γ(y)</td></tr><tr><td colspan="2">[[x⟨y⟩]]WY = ({{b ◇ b&#x27;},b&#x27;), where b = γ(y), b&#x27; = newBV(), ◇ is = if x is injective, otherwise ≥</td></tr></table>


Fig. 6. The word-wise differential denotational semantic rules for expressions.


State. A state $\gamma$ is a mapping from array entries $( x , i ) \in ( \mathbb { X } _ { f } \setminus \mathbb { K } _ { f } ) \times \mathbb { N }$ to Boolean variables that model the differences of array entries under two executions. For each variable $x \in \mathbb { X } _ { f } \ \backslash \ \mathbb { K } _ { f }$ , 

• $\gamma ( \boldsymbol { x } )$ gives the sequence of Boolean variables $\gamma ( x , 0 ) , \cdot \cdot \cdot , \gamma ( x , | x | - 1 )$ of the array ??. 

• $\gamma [ ( \boldsymbol { x } , i ) \mapsto b ]$ denotes the update of $\gamma$ by mapping $( x , i )$ to the Boolean variable $b ,$ and $\gamma [ \boldsymbol { x } \mapsto \vec { b } ]$ denotes the update $\gamma [ ( x , 0 ) \mapsto { \vec { b } } _ { 0 } ] \cdot \cdot \cdot [ ( x , | x | - 1 ) \mapsto { \vec { b } } _ { | x | - 1 } ]$ for a Boolean vector $\vec { b }$ with $\vert \vec { b } \vert = \vert x \vert$ 

By abuse of notation, $\gamma ( \boldsymbol { x } )$ gives the vector $\vec { 0 }$ with $| { \vec { 0 } } | = | x |$ if ?? is a constant or subkey variable in $\mathbb { K } _ { f }$ . Note that $\gamma ( \boldsymbol { x } )$ is $\gamma ( \boldsymbol { x } , 0 )$ if ?? has type uint?? [1]. We denote by Γ the set of states. 

Denotational semantics. The (word-wise differential) denotational semantics of an expression ?? is given by $\ [ e ] \ ] ^ { w }$ that maps each state $\gamma \in \Gamma$ to a pair $( \Psi , \vec { b } )$ , denoted by $\mathbb { I } e \mathbb { I } _ { \gamma } ^ { \mathsf { w } }$ , where 

• Ψ is a set of IL constraints over Boolean variables characterizing the dependency/feasiblity of the differences between the support variables and result of ?? such that the differences is a solution of Ψ iff these differences are feasible for support variables and result of $e ;$ 

• $\vec { b }$ such that $| \vec { b } | = | e |$ is a vector of the Boolean variables/values that models the difference of the result of ?? under two executions $( \vec { b }$ may be written as ?? if $| { \vec { b } } | = 1$ and $\vec { b } _ { 0 } = b )$ . 

Denotational semantic rules. The semantics for expressions in EasyBC is shown in Figure $^ { 6 , }$ , where the function newBV() returns a fresh Boolean variable ?? or a vector $\vec { b }$ of fresh Boolean variables according to the context. The semantic rules $[ \mathsf { I } \sim x ] | _ { \gamma } ^ { \mathsf { w } } , [ [ \mathsf { v i e w } ( x , i , j ) ] ] \mathsf { \Gamma } _ { \gamma } ^ { \mathsf { w } }$ and $\mathbb { I } \boldsymbol { x } \langle \cdot \boldsymbol { y } \cdot \rangle \mathbb { I } _ { \gamma } ^ { \mathsf { W } }$ are straightforward according to their operational semantics. We explain the others below. 

• $\left. \boldsymbol { x } _ { 1 } \odot \boldsymbol { x } _ { 2 } \right. _ { \gamma } ^ { \mathsf { W } }$ for $\odot \in \{ + , - , \oplus , \wedge , \vee \}$ gives a pair $\left( \Psi ( b _ { 0 } , b _ { 1 } , b _ { 2 } ) , b _ { 0 } \right)$ , where $\Psi ( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ characterizes the dependency of the differences $b _ { 0 } , b _ { 1 }$ and $b _ { 2 }$ between $x _ { 1 } \odot x _ { 2 } , x _ { 1 }$ and $x _ { 2 }$ according to the maximum and minimum word-wise branch numbers of $\odot$ (cf. Definition 2.7), and $b _ { 0 } = 0$ if $b _ { 1 } = b _ { 2 } = 0$ $\mathcal { B } _ { \mathsf { w w } , + } ^ { \operatorname* { m i n } } = 2$ $\mathcal { B } _ { \mathsf { w w } , + } ^ { \mathrm { m a x } } = 3$ $b _ { 0 } = b _ { 1 } = b _ { 2 } = 0$ or at least two of them are 1 (i.e., $2 \leq b _ { 0 } + b _ { 1 } + b _ { 2 } \leq 3 )$ . For easy reference, these IL constraints are denoted by $\Psi _ { 2 , 3 } ^ { 1 }$ or $\Psi _ { 2 , 3 } ^ { 2 }$ . Note that the auxiliary Boolean variable $b ^ { \prime }$ in $\Psi _ { 2 , 3 } ^ { 2 }$ is 0 iff $b _ { 0 } + b _ { 1 } + b _ { 2 } = 0$ 

• $[ \boldsymbol { M } * \boldsymbol { x } ] _ { \gamma } ^ { \mathsf { w } }$ gives the pair $\left( \Psi _ { M } ^ { i } ( \vec { b } , \vec { b ^ { \prime } } ) , \vec { b ^ { \prime } } \right)$ , where $\Psi _ { M } ^ { i } ( \vec { b } , \vec { b ^ { \prime } } )$ characterizes the dependency of the differences $\vec { b }$ and $\vec { b ^ { \prime } }$ between the entries in the arrays ?? and $M \odot x$ according to the maximum and minimum word-wise branch numbers of the linear transformation $M \odot x .$ Indeed, $\Psi _ { M } ^ { i } ( \vec { b } , \vec { b ^ { \prime } } )$ $\begin{array} { r } { \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) } \end{array}$ 

$$
\begin{array}{c c c} \hline \frac {}{\llbracket \tau   x \rrbracket_ {\gamma} ^ {\mathsf {W}} = (\emptyset , \gamma , \emptyset)} & \frac {\gamma^ {\prime} = \gamma [ (x , i) \mapsto \gamma (y) ]}{\llbracket x [ i ] = y \rrbracket_ {\gamma} ^ {\mathsf {W}} = (\emptyset , \gamma^ {\prime} , \emptyset)} & \frac {\llbracket e \rrbracket_ {\gamma} ^ {\mathsf {W}} = (\Psi , \vec {b}) \qquad \gamma^ {\prime} = \gamma [ x \mapsto \vec {b} ]}{\Theta = \left(e \text {is} x ^ {\prime} \langle y \rangle ?   \gamma (y) : \emptyset\right)} \\ \hline \tau_ {0} f (\tau_ {1} x _ {1}, \dots , \tau_ {m} x _ {m}) \{S _ {1}; \dots ; S _ {n}; \texttt {r e t u r n} y; \} & & \gamma_ {0} = \gamma [ x _ {1} \mapsto \gamma (y _ {1}) ] \dots [ x _ {m} \mapsto \gamma (y _ {m}) ] \\ \llbracket S _ {1} \rrbracket_ {\gamma_ {0}} ^ {\mathsf {W}} = (\Psi_ {1}, \gamma_ {1}, \Theta_ {1}), \dots , \llbracket S _ {n} \rrbracket_ {\gamma_ {n - 1}} ^ {\mathsf {W}} = (\Psi_ {n}, \gamma_ {n}, \Theta_ {n}) & \Psi = \bigcup_ {i = 1} ^ {n} \Psi_ {i} & \gamma^ {\prime} = \gamma [ x \mapsto \gamma_ {n} (y) ] \quad \Theta = \bigcup_ {i = 1} ^ {n} \Theta_ {i} \\ \hline & \llbracket x = g (y _ {1}, \dots , y _ {m}) \rrbracket_ {\gamma} ^ {\mathsf {W}} = (\Psi ,   \gamma^ {\prime}, \Theta) \\ \end{array}
$$

Fig. 7. The word-wise differential denotational semantic rules for statements. 

Bmin $\mathcal { B } _ { \mathsf { w w } , M } ^ { \operatorname* { m i n } }$ to Bmax $\mathcal { B } _ { \mathsf { w w } , M } ^ { \mathrm { m a x } }$ ww,?? or is 0 in two alternative ways $\Psi _ { M } ^ { 1 } ( \vec { b } , \vec { b ^ { \prime } } )$ and $\Psi _ { M } ^ { 2 } ( \vec { b } , \vec { b ^ { \prime } } )$ , where the auxiliary $b ^ { \prime \prime }$ $\begin{array} { r } { \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) = 0 } \end{array}$ $\mathcal { B } _ { \mathsf { w w } , M } ^ { \operatorname* { m i n } } \geq 1$ 

• $\mathbb { I } ^ { \boldsymbol { x } \langle \boldsymbol { y } \rangle \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { W } } }$ for S-box ?? depends upon whether ?? is injective, which is determined by checking whether some constant in the array appears more than once if the S-box is given by an array, or by checking the satisfiability of the constraint $x \neq x ^ { \prime } \land f ( x ) = f ( x ^ { \prime } )$ (via SMT solving) if the S-box is defined by an $s \_ f { \mathsf n }$ function $f .$ . If it is injective, $\mathbb { I } ^ { \boldsymbol { x } \langle \boldsymbol { y } \rangle \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \boldsymbol { w } } }$ gives the pair $\left( \{ b = b ^ { \prime } \} , b ^ { \prime } \right)$ , where the Boolean variable ?? models the difference of ??; the fresh Boolean variable $b ^ { \prime }$ models the difference of the result $x \langle y \rangle$ ; and the constraint $\boldsymbol { b } = \boldsymbol { b } ^ { \prime }$ ensures that $b = 1$ iff $b ^ { \prime } = 1$ . If it is non-injective, the constraint $b \geq b ^ { \prime }$ is imposed instead of $b = b ^ { \prime }$ , as $x \langle y \rangle$ may differ in two executions only if ?? differs in the two executions. 

Lemma 5.1. Suppose $\mathbb { [ } e \mathbb { J } _ { \gamma } ^ { \mathsf { w } } = ( \Psi , \vec { b } )$ with $\Psi \neq \emptyset . \vec { b } = ( b _ { 1 } , \cdot \cdot \cdot , b _ { m } )$ is a solution of Ψ if and only $i f ( b _ { 1 } , \cdots , b _ { i } )$ is feasible differences of the operands and result of ??, where $( b _ { i + 1 } , \cdot \cdot \cdot , b _ { m } )$ is for the possible auxiliary Boolean variables. 

## 5.2 Word-wise Differential Denotational Semantics for Statements

Denotational semantics for statements. The (word-wise differential) denotational semantics of a statement ?? is given by $[ [ S ] ] ^ { w }$ that maps each state $\gamma \in \Gamma$ to a triple $( \Psi , \gamma ^ { \prime } , \Theta )$ , denoted by $\mathbb { I } ^ { S } \mathbb { I } _ { \gamma } ^ { w } ,$ where Ψ is defined as above (i.e., set of IL constraints), $\gamma ^ { \prime }$ is the updated state, and Θ is a set of Boolean variables each of which models the input difference of an S-box under two executions. 

Denotational semantic rules. The semantics for statements in EasyBC is shown in Figure 7. 

The semantic rules $[ \tau \boldsymbol { x } ]  | _ { \gamma } ^ { \mathsf { w } } , [ [ \boldsymbol { x } [ i ] = \boldsymbol { y } ] ] _ { \gamma } ^ { \mathsf { w } }$ and $\mathbb { I } \boldsymbol { x } = e \mathbb { I } _ { \boldsymbol { Y } } ^ { \boldsymbol { w } }$ for declaration $\tau \ : x ,$ array put $x [ i ] = y ,$ assignment $x = e$ are straightforward, which updates the state $\gamma$ accordingly to track the mapping from variables ?? to Boolean variables $\gamma ( \boldsymbol { x } )$ that models the difference of ?? under two executions, the set of constraints Ψ is collected from the semantics $\mathbb { I } e \mathbb { I } _ { \gamma } ^ { \mathsf { w } }$ of the expression $e ,$ and moreover, the Boolean variable $\gamma ( y )$ modeling the difference of the input ?? of an S-box is recorded in Θ. 

The semantic rule $\mathbb { I } ^ { \ b { x } = \ b { g } ( \ b { y } _ { 1 } , \dots , \ b { y } _ { m } ) \mathbb { I } _ { \ b { \gamma } } ^ { \ b { w } } }$ for a function call $x = g ( y _ { 1 } , \cdot \cdot \cdot , y _ { m } )$ follows its operational semantics. We first pass the Boolean variables $\gamma ( y _ { i } )$ for $1 \leq i \leq m$ that model the differences of the actual arguments $y _ { i }$ to the formal parameters $x _ { i }$ , then iteratively evaluate each statement $S _ { i }$ in its function body, and finally maps the variable ?? to the Boolean variable $\gamma ( y )$ that models the difference of the return ??. The set $\Psi$ of IL constraints and the set Θ of Boolean variables modeling the input differences of S-boxes are collected from them of the statements, i.e., $\Psi _ { i }$ ’s and $\Theta _ { i } { ' } s$ 

## 5.3 Word-wise Resistance Evaluation

To evaluate the resistance of the program ??, we define the semantics $\mathbb { [ } P \mathbb { ] } ^ { \mathsf { w } }$ of the program ?? as the semantics of its fn function as follows: 

$$
\llbracket P \rrbracket^ {\mathsf {W}} = \llbracket \text {uints} [ n ] f (\text {uints} [ n _ {1} ] k, \text {uints} [ n ] t x t) \{S _ {1}; \dots ; S _ {n}; \text {return} y; \} \rrbracket^ {\mathsf {W}} = (\Psi , \gamma_ {n}, \Theta)
$$

where $\begin{array} { r } { \Psi = \bigcup _ { i = 1 } ^ { n } \Psi _ { i } , \Theta = \bigcup _ { i = 1 } ^ { n } \Theta _ { i } , \left[ S _ { 1 } \right] _ { Y _ { 0 } } ^ { \mathbb { W } } = ( \Psi _ { 1 } , \gamma _ { 1 } , \Theta _ { 1 } ) , \cdots , \left[ S _ { n } \right] _ { Y _ { n - 1 } } ^ { \mathbb { W } } = ( \Psi _ { n } , \gamma _ { n } , \Theta _ { n } ) } \end{array}$ and $\gamma _ { 0 }$ is an initial state mapping each array element of the formal parameter ?????? to a fresh Boolean variable. 

Clearly, Φ is the set of IL constraints imposed by the dependency of differences between support variables and results of all the operations in the program $P ,$ and $\sum _ { b \in \Theta } b$ gives the sum of the number of active S-boxes under two executions of the program ??. Determining the lower bound of the minimum number of active S-boxes under two executions of ?? amounts to minimizing $\sum _ { b \in \Theta } b$ subject to $\begin{array} { r } { \Phi \cup \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq 1 \} } \end{array}$ , where $\begin{array} { r } { \big ( \sum _ { i = 0 } ^ { n - 1 } \gamma \big ( t x t , i \big ) \big ) \geq 1 } \end{array}$ ensures that the input difference of text ?????? is nonzero, otherwise $\sum _ { b \in \Theta } b$ would trivially be 0. 

Recall from Section 3.5 that $N _ { \mathsf { d i f f } }$ denotes the minimum number of active S-boxes in all the possible pairs of executions. 

Theorem 5.2. $L e t \left[ \left[ P \right] \right] ^ { \mathsf { W } } = \left( \Phi , \gamma , \Theta \right)$ ) and ?? be the minimum value of the objective function $\sum _ { b \in \Theta } b$ subject to Φ $\cup \left\{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq 1 \right\}$ . We have that $N \leq N _ { \mathsf { d i f f } }$ . 

When the MILP program cannot be solved efficiently with large round number ??, one can turn to the following decomposition approach. 

Proposition 5.3. Let $n _ { 1 }$ and $n _ { 2 }$ be the minimum number of the active S-boxes of the first $r _ { 1 } - r o u n d$ and the subsequent ??2-round differential characteristics respectively, then $n _ { 1 } + n _ { 2 }$ is a lower bound of the minimum number of the active S-boxes of the $\left( r _ { 1 } + r _ { 2 } \right)$ -round differential characteristics. 

In practice, one may be only interested in proving the resistance against differential cryptanalysis $\begin{array} { r } { \dot { N } _ { \mathrm { d i f f } } \geq \frac { - \mathcal { G } } { \log _ { 2 } { p } } } \end{array}$ , where $\boldsymbol { p }$ denotes the maximum probability $\mathsf { P r } _ { S } ( \Delta X , \Delta Y )$ among all the nonzero differentials $\bar { ( \Delta X , \Delta Y ) }$ for any $S -$ box S that is active in ??-round differential characteristics and $\mathcal { \mathcal { Q } }$ is the block size of the cipher. By Theorem 5.2, we only need to verify if $\textstyle \{ \sum _ { b \in \Theta } b < { \frac { - \ell } { \log _ { \mathcal { P } } { \mathcal { P } } } } \} \cup \Phi \cup \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq 1 \}$ 

Corollary 5.4. Let $\begin{array} { r } { [ P ] ^ { \mathbb { V } } ~ = ~ ( \Phi , \gamma , \Theta ) . ~ I f \left\{ \sum _ { b \in \Theta } b ~ < ~ \frac { - \bar { \mathcal { E } } } { \log _ { 2 } \bar { p } } \right\} \cup \Phi \cup \big \{ \big ( \sum _ { i = 0 } ^ { n - 1 } \gamma \big ( t x t , i \big ) \big ) ~ \ge ~ 1 \big \} } \end{array}$ < −??log ?? } ∪ Φ ∪ {(Í??−1??=0 ?? (??????, ??)) ≥ 1} is unsatisfiable, then the program ?? with block size ?? is resistant against differential cryptanalysis. 

If the ??-round cipher ?? can be partitioned into $\frac { s } { s ^ { \prime } }$ identical $s ^ { \prime } .$ -round ciphers $P ^ { \prime }$ , by Proposition 5.3 and Corollary 5.4, we can conclude that the cipher ?? is resistant against differential cryptanalysis if the number of active S-boxes of the cipher $P ^ { \prime }$ is no less than $\frac { - s ^ { \prime } \breve { \cdot } \mathscr { Q } } { s \cdot \log _ { 2 } { p } }$ . 

Corollary 5.5. Let $\begin{array} { r } { \big [ \boldsymbol { P } ^ { \prime } \big ] ^ { \mathbb { M } } = ( \Phi , \boldsymbol { \gamma } , \Theta ) . \boldsymbol { I } f \{ \sum _ { b \in \Theta } b < \frac { - s ^ { \prime } \cdot \hat { \varepsilon } } { s \cdot \log _ { \mathcal { P } } \hat { \varepsilon } } \} \cup \Phi \cup \{ \big ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) \big ) \geq 1 \} } \end{array}$ is unsatisfiable, then the program ?? with block size $\mathcal { \mathcal { Q } }$ is resistant against differential cryptanalysis. 

## 6 BIT-WISE APPROACH

In this section, we present a bit-wise approach which, as the word-wise approach, determines the lower bound of the minimum number of active S-boxes, but lifts its limitation requiring that a program uses types uint?? [??] for a fixed bit size ?? of involved S-boxes and individual bits of any entry in an array cannot be changed. 

High-level intuition. Fix a program ??, which we normally assume cannot be handled by the wordwise approach. A straightforward idea is to transform the program $P$ to its Boolean counterpart $P ^ { \prime }$ by bit-blasting. However, this would introduce a large number of variables and statements, resulting in a prohibitively large MILP problem. In this work, we adopt a strategy to “implicitly” bit-blast, meaning that bit-blasting is performed in the generation of MILP. To this end, each bit of a variable in the program ?? is modeled by one Boolean variable ??, where $b = 1$ in an MILP solution indicates that the corresponding bit differs in ?? when executed under two distinct inputs for some fixed key. Thus, a variable of type uint?? [??] is modeled by a vector $\vec { b }$ of ${ \mathit { \Sigma } } _ { s } \cdot n$ Boolean variables. The word-wise differential denotational semantics is then lifted to the bit-wise one. 

## 6.1 Bit-wise Differential Denotational Semantics for Expressions

State. We first lift the state ?? from word-wise to bit-wise. Let $\left\| { \boldsymbol { x } } \right\| = s \cdot n$ for a variable ?? of the type uint?? [??]. A state ?? now maps each pair $( x , i ) \in ( \mathbb { X } _ { f } \setminus \mathbb { K } _ { f } ) \times \mathbb { N }$ to a Boolean variable, where 

• for each variable ?? of type uint?? [??], $\gamma ( \boldsymbol { x } , i \cdot \boldsymbol { s } + j )$ gives a Boolean variable modeling the difference of the ( ?? + 1)-th most significant bit of the $( i + 1 )$ -th entry $x _ { i }$ in the array ??; 

• ?? (??) denotes the sequence $\gamma ( x , 0 ) , \gamma ( x , 1 ) , \cdots , \gamma ( x , \| x \| - 1 ) , \gamma [ ( x , i ) \mapsto b ]$ denotes the update of the state ?? by mapping $( x , i )$ to the Boolean variable $b ,$ and $\gamma [ \boldsymbol { x } \mapsto \vec { b } ]$ denotes the update $\gamma [ ( x , 0 ) \mapsto { \vec { b } } _ { 0 } ] \cdot \cdot \cdot [ ( x , \| x \| - 1 ) \mapsto { \vec { b } } _ { \| x \| - 1 } ]$ for a Boolean vector $\vec { b }$ with $\| x \| = \| { \vec { b } } \|$ . 

Denotational semantics. The (bit-wise differential) denotational semantics of an expression ?? is given by $[ [ e ] ] ^ { { \mathsf { B } } }$ that maps each state $\gamma \in \Gamma$ to a pair $( \Psi , \vec { b } )$ , denoted by $\mathbb { I } e \mathbb { I } _ { \gamma } ^ { \mathsf { B } }$ , where 

• Ψ is a set of IL constraints over Boolean variables characterizing the bit-level dependency of differences between the support variables and result of ?? such that the differences are a solution of Ψ iff these bit-level differences are feasible for support variables and result of $e ;$ 

• $\vec { b }$ such that $\vert \vert \vec { b } \vert \vert = \vert \vert e \vert \vert$ is a Boolean vector modeling the bit-level differences of the result of ??. 

Denotational semantic rules. The semantics for expressions in EasyBC is shown in Figure 8. The semantic rules $[ \mathsf { I } \sim x ] ] _ { Y } ^ { \mathsf { B } } , [ \mathsf { I } \lor \mathsf { i e w } ( x , i , j ) ] ] _ { Y : \quad Y } ^ { \mathsf { B } }$ , ⟦touint $\left( x _ { 1 } , \cdots , x _ { m } \right) \big ] \big | _ { \gamma } ^ { \mathsf { B } }$ and $\mathbb { I } ^ { \boldsymbol { x } \langle \cdot \boldsymbol { y } \cdot \rangle } \mathbb { I } _ { \gamma } ^ { \mathsf { w } }$ are trivial according to their operational semantics. Below, we explain the other non-trivial ones. 

• The semantic rule $\mathbb { I } \boldsymbol { x } \odot \boldsymbol { y } \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { B } }$ for $\odot \in \{ \land , \lor \}$ gives the pair $\big ( \{ \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq \vec { b } _ { i } ^ { 0 } \ | \ 0 \leq i < \| x \| \} , \vec { b } ^ { 0 } \big )$ , where for every $0 \leq i < \| x \| , { \vec { b } } _ { i } ^ { 1 } + { \vec { b } } _ { i } ^ { 2 } \geq { \vec { b } } _ { i } ^ { 0 }$ characterizes that if the (?? + 1)-th bits of the operands ?? and ?? have no differences $( \mathrm { i . e . , } \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0 )$ , then the (?? + 1)-th bit of the result $x \odot y$ has no differences $( \mathrm { i . e . , } \vec { b } _ { i } ^ { 0 } = 0 )$ . Otherwise, it may have differences (with the probability of $\scriptstyle { \frac { 1 } { 2 } } )$ . 

$\mathbb { I } \boldsymbol { x } \oplus \boldsymbol { y } \mathbb { I } _ { \gamma } ^ { \mathsf { B } }$ $\begin{array} { r } { \big ( \bigcup _ { i = 0 } ^ { \| x \| - 1 } \psi _ { \oplus } ^ { j } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } ) , \vec { b } ^ { 0 } \big ) } \end{array}$ $0 \leq i < \| x \|$ $\psi _ { \oplus } ^ { j } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ characterizes that either the (?? + 1)-th bits of the operands ??, ?? and result $x \oplus y$ have no differences (i.e., $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0 )$ or exactly two of them have differences $( \mathrm { i . e . , } \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } = 2 )$ . 

• The semantic rule $[ [ x \odot y ] ] _ { \gamma } ^ { \mathsf { B } }$ for $\odot \in \{ + , - \}$ gives the pair $\textstyle \big ( \bigcup _ { i = 0 } ^ { \| x \| - 1 } \Psi _ { i } , { \vec { b } } ^ { 0 } \big )$ , where $\Psi _ { i }$ characterizes the dependency of the differences between the ??-th and (?? + 1)-th bits of the operands ??, ?? and result $x \odot y .$ . The dependency is obtained by bit-blasting ?? + ?? via a ripple-carry adder, i.e., 

$- \ \mathrm { b i n } _ { i } ( x + y ) = \mathrm { b i n } _ { i } ( x )$ ⊕ $\mathbf { b i n } _ { i } ( y ) \oplus { \vec { c } } _ { i }$ , for every $0 \leq i < \| x \| ;$ 

– the carry bit ${ \vec { c } } _ { i } = 1 { \mathrm { ~ i f f ~ b i n } } _ { i - 1 } ( x ) + \mathrm { b i n } _ { i - 1 } ( y ) + { \vec { c } } _ { i - 1 } \geq 2 $ , for every $1 \leq i < \| x \|$ , with ${ \vec { c } } _ { 0 } = 0$ 

where $\mathsf { b i n } _ { i } ( x )$ denotes the (?? + 1)-th most significant bit of ??. Clearly, the difference $\vec { b } _ { i } ^ { 0 }$ of the bit bi $\mathfrak { r } _ { i } ( x + y )$ depends upon the differences $\vec { b } _ { i - 1 } ^ { 1 } , \vec { b } _ { i - 1 } ^ { 2 } , \vec { b } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ of the bits bi $\mathfrak { r } _ { i - 1 } ( x )$ , $\mathsf { i n } _ { i - 1 } ( y )$ bin?? (??) and $\mathsf { b i n } _ { i } ( y )$ . Indeed, we can deduce the dependency: 

$- \ \mathrm { i f } \ \vec { b } _ { i - 1 } ^ { 0 } = \vec { b } _ { i - 1 } ^ { 1 } = \vec { b } _ { i - 1 } ^ { 2 } = 1$ , then $\vec { c } _ { i - 1 } \oplus \vec { c } _ { i - 1 } ^ { \prime } = \vec { b } _ { i - 1 } ^ { 3 } = \vec { b } _ { i } ^ { 3 } = 1$ and $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ 

$- \ \mathrm { i f } \ \overrightarrow { b } _ { i - 1 } ^ { 0 } = \overrightarrow { b } _ { i - 1 } ^ { 1 } = \overrightarrow { b } _ { i - 1 } ^ { 2 } = 0 ,$ , then $\vec { c } _ { i - 1 } \oplus \vec { c } _ { i - 1 } ^ { \prime } = \vec { b } _ { i - 1 } ^ { 3 } = \vec { b } _ { i } ^ { 3 } = 0$ and $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 }$ 

– otherwise $1 \leq \vec { b } _ { i - 1 } ^ { 0 } + \vec { b } _ { i - 1 } ^ { 1 } + \vec { b } _ { i - 1 } ^ { 2 } \leq 2$ . Indeed, the probability of $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ , (resp. $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 }$ and $\vec { b } _ { i } ^ { 0 } = 1 ) \mathrm { ~ i s ~ } \frac { 1 } { 2 }$ . 

The above dependency is characterized by the set of IL constraints $\Psi _ { i }$ . Furthermore, $\Psi _ { 0 }$ and $\Psi _ { 1 }$ can be simplified by ${ \vec { c } } _ { 0 } = 0$ . The semantic rule $\mathbb { I } \boldsymbol { x } - \boldsymbol { y } \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { B } }$ is defined the same as $[ \boldsymbol { x } + \boldsymbol { y } ] ] _ { \gamma } ^ { \mathsf { B } }$ because ?? = ?? − ?? iff $x = z + y$ , and the Boolean variables $\vec { b } _ { i } ^ { 0 } , \vec { b } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ in $\Psi _ { i }$ are symmetric. 

$\mathbb { I } M * x \mathbb { I } _ { \gamma } ^ { \mathsf { B } }$ $( \cup _ { i = 0 } ^ { | x | - 1 } \bigcup _ { h = 0 } ^ { s - 1 } \Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { v } , \vec { b ^ { \prime } } )$ $0 \leq i < | x |$ $0 \leq h < s , \Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { v }$ array ?? and the (ℎ + 1)-th bit $\mathsf { b i n } _ { h } ( y _ { i } )$ of the (?? + 1)-th entry $y _ { i }$ in the resulting array $y = M * x$ 

<table><tr><td><eq>\llbracket \sim x \rrbracket_{\gamma}^{\mathsf{B}} = (\emptyset, \gamma(x))</eq></td><td><eq>\llbracket \text{touint}(x_1, \cdots, x_m) \rrbracket_{\gamma}^{\mathsf{B}} = (\emptyset, (\gamma(x_1), \cdots, \gamma(x_m)))</eq></td></tr><tr><td colspan="2"><eq>\llbracket \text{View}(x, i, j) \rrbracket_{\gamma}^{\mathsf{B}} = (\emptyset, (\vec{b}_{i \cdot s+0}, \cdots, \vec{b}_{i \cdot s+s-1}, \cdots, \vec{b}_{j \cdot s+0}, \cdots, \vec{b}_{j \cdot s+s-1}))</eq>, where <eq>\vec{b} = \gamma(x)</eq></td></tr><tr><td colspan="2"><eq>\llbracket x \wedge y \rrbracket_{\gamma}^{\mathsf{B}} = \llbracket x \vee y \rrbracket_{\gamma}^{\mathsf{B}} = (\{\vec{b}_i^1 + \vec{b}_i^2 \geq \vec{b}_i^0 \mid 0 \leq i &lt; \|x\|\}, \vec{b}^0)</eq>, where <eq>\vec{b}^1 = \gamma(x)</eq>, <eq>\vec{b}^2 = \gamma(y)</eq>, <eq>\vec{b}^0 = \text{newBV}()</eq></td></tr><tr><td colspan="2"><eq>\llbracket x \oplus y \rrbracket_{\gamma}^{\mathsf{B}} = (\bigcup_{i=0}^{\|x\| - 1} \psi_\oplus^j(\vec{b}_i^1, \vec{b}_i^2, \vec{b}_i^0), \vec{b}^0)</eq>, where <eq>\vec{b}^1 = \gamma(x)</eq>, <eq>\vec{b}^2 = \gamma(y)</eq>, <eq>\vec{b}^0 = \text{newBV}()</eq>, <eq>b&#x27; = \text{newBV}()</eq><eq>\psi_\oplus^1(b^1, b^2, b^0) = \{2 \geq b^0 + b^1 + b^2 \geq 2b&#x27;, b&#x27; \geq b^0, b&#x27; \geq b^1, b&#x27; \geq b^2\}</eq> [Sun et al. 2014a]<eq>\psi_\oplus^2(b^1, b^2, b^0) = \{b^0 + b^1 + b^2 \leq 2, b^1 + b^2 \geq b^0, b^0 + b^1 \geq b^2, b^0 + b^2 \geq b^1\}</eq> [Sasaki and Todo 2017]<eq>\psi_\oplus^3(b^1, b^2, b^0) = \{b^0 + b^1 + b^2 = 2b&#x27;\}</eq> [Cui et al. 2016]</td></tr><tr><td colspan="2"><eq>\llbracket x + y \rrbracket_{\gamma}^{\mathsf{B}} = \llbracket x - y \rrbracket_{\gamma}^{\mathsf{B}} = (\bigcup_{i=0}^{\|x\| - 1} \Psi_i, \vec{b}^0)</eq>, where <eq>\vec{b}^1 = \gamma(x)</eq>, <eq>\vec{b}^2 = \gamma(y)</eq>, <eq>\vec{b}^0 = \text{newBV}()</eq>,<eq>\Psi_0 = \Psi_\oplus^i(\vec{b}_0^0, \vec{b}_0^1, \vec{b}_0^2) \quad \Psi_1 = \begin{cases} \vec{b}_1^0 + \vec{b}_1^1 + \vec{b}_1^2 \leq \vec{b}_0^1 + \vec{b}_0^2 + 2, &amp; -\vec{b}_1^0 + \vec{b}_1^1 - \vec{b}_1^2 \leq \vec{b}_0^1 + \vec{b}_0^2, \\ -\vec{b}_1^0 - \vec{b}_1^1 + \vec{b}_1^2 \leq \vec{b}_0^1 + \vec{b}_0^2, &amp; \vec{b}_1^0 - \vec{b}_1^1 - \vec{b}_1^2 \leq \vec{b}_0^1 + \vec{b}_0^2 \end{cases}</eq><eq>\forall 2 \leq i &lt; \|x\|. \Psi_i = \begin{cases} 4 \geq \sum_{j=0}^2 \vec{b}_{i-1}^j - \vec{b}_i^0 + \vec{b}_i^1 + \vec{b}_i^2 \geq 0, &amp; 4 \geq \sum_{j=0}^2 \vec{b}_{i-1}^j + \vec{b}_i^0 + \vec{b}_i^1 - \vec{b}_i^2 \geq 0, \\ 4 \geq \sum_{j=0}^2 \vec{b}_{i-1}^j + \vec{b}_i^0 - \vec{b}_i^1 + \vec{b}_i^2 \geq 0, &amp; \sum_{j=0}^2 \vec{b}_{i-1}^j + 2 \geq \vec{b}_i^0 + \vec{b}_i^1 + \vec{b}_i^2 \geq \sum_{j=0}^2 \vec{b}_{i-1}^j - 2 \end{cases}</eq></td></tr><tr><td colspan="2"><eq>\llbracket M * x \rrbracket_{\gamma}^{\mathsf{B}} = (\bigcup_{i=0}^{|x| - 1} \bigcup_{h=0}^{s-1} \Psi_{M,i,h}^v, \vec{b}&#x27;)</eq>, where <eq>\vec{b} = \gamma(x)</eq>, <eq>\vec{b}&#x27; = \text{newBV}()</eq>, <eq>b_{\text{new}}^i = \text{newBV}()</eq>, <eq>v \in \{1, 2\}</eq><eq>\Psi_{M,i,h}^v = \psi_\oplus^v(b^0, b^1, b_{\text{new}}^1) \cup \psi_\oplus^v(b_{\text{new}}^1, b^2, b_{\text{new}}^2) \cup \cdots \cup \psi_\oplus^v(b_{\text{new}}^{m-2}, b^{m-1}, b_{\text{new}}^{m-1}) \cup \psi_\oplus^v(b_{\text{new}}^{m-1}, b^m, \vec{b}_{i \cdot s+h}&#x27;)</eq><eq>\Psi_{M,i,h}^3 = \{\vec{b}_{i \cdot s+h}&#x27; + \sum_{j=0}^m b^j = 2d, 0 \leq d \leq \lfloor \frac{m+2}{2} \rfloor\}</eq>, where <eq>d</eq> is a fresh integer variable, <eq>\{b^0, \cdots, b^m\}</eq> is the set of support variables of <eq>\left( \bigoplus_{0 \leq j&lt; |x|, h \leq k&lt;s, M_{i,j,k}=1} \vec{b}_{j \cdot s+k} \right) \oplus \left( \vec{c}_h \wedge \bigoplus_{0 \leq j&lt; |x|, 0 \leq k&lt; \lfloor \frac{s}{2} \rfloor} \vec{b}_{j \cdot s+2k} \right)</eq></td></tr><tr><td colspan="2"><eq>\llbracket x\langle y\rangle \rrbracket_{\gamma}^{\mathsf{B}} = (\emptyset, \vec{b}^0 || \cdots || \vec{b}|^{x-1}), \text{where } \vec{b} = \gamma(y), x \text{ is P-box } (j_0, \cdots, j_{n-1}), \text{ and } \vec{b}^i = (\vec{b}_{ji \cdot s+0}, \cdots, \vec{b}_{ji \cdot s+s-1}) \text{ for } 0 \leq i \leq |x| - 1</eq></td></tr><tr><td colspan="2"><eq>\llbracket x\langle y\rangle \rrbracket_{\gamma}^{\mathsf{B}} = (\Psi_S \cup \Psi_S^{\text{bn}}, \vec{b}&#x27;)</eq>, where <eq>\vec{b} = \gamma(y), x: \mathbb{B}^n \to \mathbb{B}^m</eq> is an S-box <eq>S, \vec{b}&#x27; = \text{newBV}(), b = \text{newBV}(),</eq>and <eq>\Psi_S^{\text{bn}} = \left\{ \mathcal{B}_{\text{bw},x}^{\text{min}} \cdot b \leq \sum_{i=0}^{n-1} \vec{b}_i + \sum_{i=0}^{m-1} \vec{b}_i&#x27; \leq \mathcal{B}_{\text{bw},x}^{\text{max}}, b \geq \vec{b}_i, b \geq \vec{b}_j&#x27; \mid 0 \leq i &lt; n, 0 \leq j &lt; m\}</eq></td></tr></table>


Fig. 8. The bit-wise differential denotational semantic rules for expressions, where $s = { \frac { \| x \| } { | x | } }$ denotes the bit width of the entries of the array $x ,$ and ${ \vec { c } } = \mathrm { b i n } ( 2 \otimes 2 ^ { s - 1 } )$ is the ??-bitstream corresponding to the coefficients of the irreducible polynomial for the underlying finite-field.


The dependency is obtained by expanding the matrix-vector product as follows: 

$$
M * x = \big (\bigoplus_ {j = 0} ^ {| x | - 1} (M _ {0, j} \otimes x _ {j}), \dots , \bigoplus_ {j = 0} ^ {| x | - 1} (M _ {| x | - 1, j} \otimes x _ {j}) \big).
$$

$y _ { i }$ $\textstyle \bigoplus _ { j = 0 } ^ { | x | - 1 } ( M _ { i , j } \otimes x _ { j } )$ $\vec { b } _ { i \cdot s + h } ^ { \prime }$ of its $\left( h + 1 \right)$ -th bit $\mathsf { b i n } _ { h } ( y _ { i } )$ is the parity of the differences of the $\left( h + 1 \right)$ -th bits in $M _ { i , j } \otimes x _ { j }$ for $0 \leq j < h$ . The expression $M _ { i , j } \otimes x _ { j }$ is bit-blasted by expanding the finite-field multiplication (⊗) using a series of modular left shifts, XOR operations and the coefficients $\vec { c }$ of the irreducible polynomial for the underlying finite field, where ${ \vec { c } } = \mathrm { b i n } ( 2 \otimes 2 ^ { s - 1 } )$ . We finally can deduce that the parity of the differences of the $( h + 1 ) \cdot \mathrm { t h }$ bits in $M _ { i , j } \otimes x _ { j }$ for $0 \leq j < h$ is 

$$
\Bigl (\bigoplus_ {0 \leq j <   | x |, h \leq k <   s, M _ {i, j, k} = 1} \vec {b} _ {j \cdot s + k} \Bigr) \oplus \Bigl (\vec {c} _ {h} \wedge \bigoplus_ {0 \leq j <   | x |, 0 \leq k <   \lfloor \frac {s}{2} \rfloor} \vec {b} _ {j \cdot s + 2 k} \Bigr).
$$

Let $\{ b ^ { 0 } , \cdots , b ^ { m } \}$ be the set of support variables of the above expression. We have: 

$$
\vec {b} _ {i \cdot s + h} ^ {\prime} = \bigoplus_ {t = 0} ^ {m} b _ {t},
$$

which can be alternatively characterized by $\Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { v }$ for $v \in \{ 1 , 2 , 3 \}$ 

• The semantic rule $\mathbb { I } ^ { \boldsymbol { x } \langle \boldsymbol { y } \rangle \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { W } } }$ for S-box $x = S$ gives the pair $( \Psi _ { S } \cup \Psi _ { S } ^ { \mathsf { b n } } , \vec { b } ^ { \prime } )$ , where $\Psi _ { S }$ is a set of IL constraints characterizing the bit-level dependency of differences between the input ?? and the result $S ( y )$ (cf. Section 4.2) and $\Psi _ { S } ^ { \mathsf { b n } }$ enforces that the Hamming weight of the bitstream ${ \vec { b } } \| { \vec { b ^ { \prime } } }$ ranges from Bminbw,S $\mathcal { B } _ { \mathrm { b w } , S } ^ { \mathrm { m i n } } \mathrm { ~ t o ~ } \mathcal { B } _ { \mathrm { b w } , S } ^ { \mathrm { m a x } }$ $\Psi _ { S } ^ { \flat \ n }$ is redundant, it often boosts MILP solving. 

Lemma 6.1. Suppose $[ | e ] ] _ { \gamma } ^ { \mathsf { B } } = ( \Psi , \vec { b } )$ with $\Psi \ne \emptyset$ . The assignment $( b _ { 1 } , \cdots , b _ { m } )$ is a solution of Ψ if and only if $( b _ { 1 } , \cdots , b _ { i } )$ is feasible bit-level differences of the operands and result $o f e ,$ where $( b _ { i + 1 } , \cdots , b _ { m } )$ is for the possible auxiliary Boolean variables. 

## 6.2 Bit-wise Differential Denotational Semantics for Statements

The (bit-wise differential) denotational semantics of a statement ?? is given by $[ [ S ] ] ^ { \mathsf { B } }$ that maps each state $\gamma \in \Gamma$ to a triple $( \Psi , \gamma ^ { \prime } , \Theta )$ , denoted by $\mathbb { I } ^ { S } \mathbb { I } _ { \gamma } ^ { \mathsf { B } }$ , where $\Psi , \gamma ^ { \prime }$ and Θ are the same as above. 

The bit-wise differential semantic rules for statements in EasyBC are the same as those word-wise ones except for array put and S-box access, which are given below: 

$$
\frac {\vec {b} = \gamma (y) \quad x \text {   has   type   } \mathsf {u i n t s} [ n ] \quad \gamma^ {\prime} = \gamma [ (x , i \cdot s + 0) \mapsto \vec {b} _ {0} ] \cdots [ (x , i \cdot s + s - 1) \mapsto \vec {b} _ {s - 1} ]}{[ [ x [ i ] = y ] ] _ {Y} ^ {\mathsf {B}} = (\emptyset , \gamma^ {\prime} , \emptyset)}
$$

$$
\llbracket x ^ {\prime} \langle y \rangle \rrbracket_ {\gamma} ^ {B} = (\Psi , \vec {b}) \quad \vec {b} ^ {\prime} = \gamma (y) \quad b _ {x} = \operatorname{newBV} ()
$$

$$
\Psi^ {\prime} = \Psi \cup \{\sum_ {j = 0} ^ {\| y \| - 1} \vec {b} _ {j} ^ {\prime} \geq b _ {x} \geq \vec {b} _ {i} ^ {\prime} | 0 \leq i <   \| y \| \quad \gamma^ {\prime} = \gamma [ x \mapsto \vec {b} ] \}
$$

$$
\llbracket x = x ^ {\prime} \langle y \rangle \rrbracket_ {\gamma} ^ {\mathsf {B}} = (\Psi^ {\prime}, \gamma^ {\prime}, \{b _ {x} \})
$$

Intuitively, the semantic rule $\mathbb { I } \boldsymbol { x } [ i ] = \boldsymbol { y } \mathbb { J } _ { \gamma } ^ { \mathsf { B } }$ updates the state ?? accordingly by mapping the bits of the $( i + 1 )$ -th entry in the array ?? to the Boolean variables $\vec { b }$ that model the differences of the bits of the result ??. The semantic rule $\mathbb { I } \boldsymbol { x } = \boldsymbol { x } ^ { \prime } \langle \boldsymbol { y } \rangle \mathbb { I } _ { \gamma } ^ { \mathsf { B } }$ adds a fresh Boolean variable $b _ { x }$ , where if $b _ { x } = 1$ , then the S-box is active, i.e., some bit $\vec { b } _ { i } ^ { \prime }$ that models the difference of one bit of input ?? is nonzero. 

## 6.3 Bit-wise Resistance Evaluation

To evaluate the resistance of the program ?? in a bit-wise manner, similar to $\mathbb { [ } P \mathbb { ] } ^ { \mathsf { w } }$ (cf. Section 5.3), we define the (bit-wise) semantics $\mathbb { [ } \mathring { P } \ ] ^ { { \mathsf { B } } }$ of the program ?? using its fn function ?? as follows. 

$$
\llbracket P \rrbracket^ {\mathsf {B}} = \llbracket \text {uints} [ n ] f (\text {uints} [ n _ {1} ] k, \text {uints} [ n ] t x t) \{S _ {1}; \dots ; S _ {n}; \text {return} y; \} \rrbracket^ {\mathsf {B}} = (\Psi , \gamma_ {n}, \Theta)
$$

where $\begin{array} { r } { \Psi = \bigcup _ { i = 1 } ^ { n } \Psi _ { i } , \Theta = \bigcup _ { i = 1 } ^ { n } \Theta _ { i } , \left[ \left. S _ { 1 } \right] \right| _ { \gamma _ { 0 } } ^ { \mathbb { B } } = \left( \Psi _ { 1 } , \gamma _ { 1 } , \Theta _ { 1 } \right) , \allowbreak \cdots , \left[ \left. S _ { n } \right] \right| _ { \gamma _ { n - 1 } } ^ { \mathbb { B } } = \left( \Psi _ { n } , \gamma _ { n } , \Theta _ { n } \right) } \end{array}$ and $\gamma _ { 0 }$ is an initial state mapping each bit of array elements of ?????? to a fresh Boolean variable. We get that: 

Theorem 6.2. Let $[ \boldsymbol { P } ] ] ^ { \mathsf { B } } = ( \Phi , \gamma , \Theta )$ and ?? be the minimum value of the objective function $\sum _ { b \in \Theta } b$ subject to the set of IL constraints $\Phi \cup \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \}$ . We have that $N \leq N _ { \mathsf { d i f f } }$ . 

Corollary 5.4 and Corollary 5.5 still hold when $\ [ \boldsymbol { P } \ ] ^ { \mathsf { w } } = ( \Phi , \gamma , \Theta )$ is replaced by $[ \boldsymbol { P } ] ] ^ { \mathsf { B } } = ( \Phi , \gamma , \Theta )$ . 

## 7 EXTENDED BIT-WISE APPROACH

The lower bound of the minimum number of the active S-boxes is often effective, but it may not be sufficiently tight to prove the resistance [Sun et al. 2014b], as the probabilities between input and output differences for the operations $\wedge , \vee , + , -$ and S-boxes are not fully addressed. Furthermore, the bit-wise approach is not applicable if non-linear layers are implemented in other operations than S-boxes (e.g., SIMON), or S-boxes are too large to be given as arrays (e.g., SPARKLE). In this 

$$
\begin{array}{r} \llbracket x \wedge y \rrbracket_ {\gamma} ^ {\mathsf {E B}} = \llbracket x \vee y \rrbracket_ {\gamma} ^ {\mathsf {E B}} = \big (\cup_ {i = 0} ^ {\| x \| - 1} \Psi_ {i}, \vec {b} ^ {0}, \sum_ {i = 0} ^ {\| x \| - 1} \vec {p} _ {i} \big), \mathrm{where} \vec {b} ^ {1} = \Gamma (x), \vec {b} ^ {2} = \Gamma (y), \vec {b} ^ {0} = \mathrm{newBV()} \\ \Psi_ {i} = \{\vec {b} _ {i} ^ {1} + \vec {b} _ {i} ^ {2} \geq \vec {p} _ {i}, \vec {b} _ {i} ^ {0} + \vec {b} _ {i} ^ {1} + \vec {b} _ {i} ^ {2} \leq 3 \vec {p} _ {i} \} \end{array}
$$

$$
\begin{array}{l} \llbracket x + y \rrbracket_ {\gamma} ^ {\mathsf {E B}} = \llbracket x - y \rrbracket_ {\gamma} ^ {\mathsf {E B}} = \big (\cup_ {i = 0} ^ {\| x \| - 1} \Psi_ {i},   \vec {b} ^ {0},   \sum_ {i = 1} ^ {\| x \| - 1} \vec {p} _ {i} \big), \text {where} \vec {b} ^ {1} = \Gamma (x),   \vec {b} ^ {2} = \Gamma (y),   \vec {b} ^ {0} = \operatorname{newBV} () \\ \Psi_ {0} = \Psi_ {\oplus} ^ {i} \big (\vec {b} _ {0} ^ {0}, \vec {b} _ {0} ^ {1}, \vec {b} _ {0} ^ {2} \big) \qquad \qquad \Psi_ {1} = \left\{ \begin{array}{c c c} \vec {b} _ {1} ^ {0} + \vec {b} _ {1} ^ {1} + \vec {b} _ {1} ^ {2} \leq \vec {p} _ {1} + 2, & - \vec {b} _ {1} ^ {0} + \vec {b} _ {1} ^ {1} - \vec {b} _ {1} ^ {2} \leq \vec {p} _ {1}, & - \vec {b} _ {1} ^ {0} - \vec {b} _ {1} ^ {1} + \vec {b} _ {1} ^ {2} \leq \vec {p} _ {1}, \\ \vec {b} _ {1} ^ {0} - \vec {b} _ {1} ^ {1} - \vec {b} _ {1} ^ {2} \leq \vec {p} _ {1}, & \vec {p} _ {1} \leq \vec {b} _ {0} ^ {1} + \vec {b} _ {0} ^ {2} \leq 2 \vec {p} _ {1} \end{array} \right\} \\ \forall 2 \leq i <   \| x \|.   \Psi_ {i} = \Psi_ {i} ^ {1} \cup \Psi_ {i} ^ {2} \qquad \qquad \qquad \Psi_ {i} ^ {1} = \left\{ \begin{array}{c c} 3 - \vec {p} _ {i} \geq \sum_ {j = 0} ^ {2} \vec {b} _ {i - 1} ^ {j} \geq \vec {p} _ {i}, & \vec {p} _ {i} \geq \vec {b} _ {i - 1} ^ {0} - \vec {b} _ {i - 1} ^ {1}, \\ \vec {p} _ {i} \geq \vec {b} _ {i - 1} ^ {1} - \vec {b} _ {i - 1} ^ {2}, & \vec {p} _ {i} \geq \vec {b} _ {i - 1} ^ {2} - \vec {b} _ {i - 1} ^ {0} \end{array} \right\} \\ \Psi_ {i} ^ {2} = \left\{ \begin{array}{l l} 2 - \vec {b} _ {i - 1} ^ {1} + \vec {p} _ {i} \geq - \vec {b} _ {i} ^ {0} + \vec {b} _ {i} ^ {1} + \vec {b} _ {i} ^ {2} \geq - \vec {b} _ {i - 1} ^ {1} - \vec {p} _ {i}, & 2 - \vec {b} _ {i - 1} ^ {1} + \vec {p} _ {i} \geq \vec {b} _ {i} ^ {0} + \vec {b} _ {i} ^ {1} - \vec {b} _ {i} ^ {2} \geq - \vec {b} _ {i - 1} ^ {1} - \vec {p} _ {i}, \\ 2 - \vec {b} _ {i - 1} ^ {1} + \vec {p} _ {i} \geq \vec {b} _ {i} ^ {0} - \vec {b} _ {i} ^ {1} + \vec {b} _ {i} ^ {2} \geq - \vec {b} _ {i - 1} ^ {1} - \vec {p} _ {i}, & 2 + \vec {b} _ {i - 1} ^ {1} + \vec {p} _ {i} \geq \vec {b} _ {i} ^ {0} + \vec {b} _ {i} ^ {1} + \vec {b} _ {i} ^ {2} \geq \vec {b} _ {i - 1} ^ {1} - \vec {p} _ {i} \end{array} \right\} \\ \end{array}
$$

$$
\llbracket x \langle y \rangle \rrbracket_ {Y} ^ {\mathsf {E B}} = \big (\Psi_ {\mathcal {S}} ^ {\dagger} \cup \Psi_ {\mathcal {S}} ^ {\mathsf {b n}},   \vec {b} ^ {\prime},   \sum_ {j = 1} ^ {k} t _ {i _ {j}} \cdot p _ {i _ {j}} \big), \text {   where   } x \text {   is   an   S - box   } \mathcal {S}: \mathbb {B} ^ {n} \to \mathbb {B} ^ {m}
$$

Fig. 9. The extended bit-wise differential denotational semantic rules for expressions, where ${ \vec { p } } = { \mathsf { n e w B V } } ( )$ is $\Psi _ { \oplus } ^ { i } \left( \vec { b } _ { 0 } ^ { 0 } , \vec { b } _ { 0 } ^ { 1 } , \vec { b } _ { 0 } ^ { 2 } \right)$ $i \in \{ 1 , 2 , 3 \}$ $\Psi _ { S } ^ { \mathsf { b n } }$ defined in Figure 8, $\Psi _ { S } ^ { \dagger }$ and $\textstyle \sum _ { j = 1 } ^ { k } t _ { i _ { j } } \cdot p _ { i _ { j } }$ are defined in Section 4.3. 

section, we extend the bit-wise approach to directly bound the MaxEDCP rather than by bounding the minimum number of the active S-boxes. 

High-level intuition. Apart from the encoding presented in Section 4.3 for S-boxes, we further encode the probabilities between input and output differences for the operations $\wedge , \vee , + , -$ into IL constraints using additional Boolean variables. The weighted sum $\varrho$ of these additional Boolean variables retains the probability $2 ^ { - \varrho }$ . Thus, the objective function is designed to minimize $\varrho$ instead of the number of the active S-boxes. 

## 7.1 Extended Bit-wise Differential Denotational Semantics for Expressions

Denotational semantics. The (extended bit-wise differential) denotational semantics of an expression ?? is given by $\mathbb { I } e \mathbb { J } ^ { \mathsf { E B } }$ that maps each state $\gamma \in \Gamma$ to a triple $( \Psi , \vec { b } , \varrho )$ , denoted by ${ \mathbb { I } } e { \mathbb { I } } _ { \gamma } ^ { \mathsf { E B } }$ , where the state $\gamma ,$ set of IL constraint Ψ and Boolean vector $\vec { b }$ are the same as in Section 6.1, and the expression $\varrho$ is a weighted sum of Boolean variables encoding the probability $2 ^ { - \varrho }$ . 

Denotational semantic rules. The semantic rules ${ \mathbb { I } } e { \mathbb { I } } _ { \gamma } ^ { \mathsf { E B } }$ for ∧, ∨, +, − and S-boxes are shown in Figure 9, otherwise $[ \boldsymbol { e } ] ] _ { \gamma } ^ { \mathsf { E B } } = ( \boldsymbol { \Psi } , \vec { b } , 0 ) \mathrm { i f } [ [ \boldsymbol { e } ] ] _ { \gamma } ^ { \mathsf { B } } = ( \boldsymbol { \Psi } , \vec { b } )$ , meaning that the probability between the input and output differences characterized by Ψ is $1 ( \mathrm { i . e . , } 2 ^ { - 0 } )$ . 

• The semantic rule $\mathbb { I } \boldsymbol { x } \odot \boldsymbol { y } \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { E B } }$ for $\odot \in \{ \land , \lor \}$ gives the triple $\begin{array} { r } { \big ( \bigcup _ { i = 0 } ^ { \| x \| - 1 } \Psi _ { i } , \vec { b } ^ { 0 } , \sum _ { i = 0 } ^ { \| x \| - 1 } \vec { p } _ { i } \big ) } \end{array}$ , where for every $0 \leq i < \| x \|$ , Ψ?? ensures that the probability of the difference $\vec { b } _ { i } ^ { 0 }$ of the $( i + 1 )$ -th bit in the result $x \odot y$ is $2 ^ { - { \vec { p } } _ { i } }$ when the differences of the $( i + 1 )$ -th bits of the operands ?? and ?? are $ { \vec { b } } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ , respectively. Indeed, the probability of $\vec { b } _ { i } ^ { 0 } = 0 \mathrm { i } s 2 ^ { - 0 }$ when $\vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0$ , then $\vec { p } _ { i }$ must be 0. The probability of $\vec { b } _ { i } ^ { 0 } = 1 \mathrm { i } s 2 ^ { - 1 }$ when $\vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq \vec { 1 }$ , then $\vec { p } _ { i }$ must be 1. 

• The semantic rule $\mathbb { I } \boldsymbol { x } \odot \boldsymbol { y } \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { E B } }$ for $\odot \in \{ + , - \}$ gives the triple $\begin{array} { r } { \big ( \bigcup _ { i = 0 } ^ { \| x \| - 1 } \Psi _ { i } , \vec { b } ^ { 0 } , \sum _ { i = 1 } ^ { \| x \| - 1 } \vec { p } _ { i } \big ) } \end{array}$ , where for every $0 \leq i < \| x \| , \Psi _ { i }$ ensures that for any fixed $ { \vec { b } } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ , $- \ \mathrm { i f } \ \vec { b } _ { i - 1 } ^ { 0 } = \vec { b } _ { i - 1 } ^ { 1 } = \vec { b } _ { i - 1 } ^ { 2 } , \vec { p } _ { i } = 0 \ \mathrm { ( i . e . }$ , the probability $2 ^ { - { \vec { p } } _ { i } }$ of $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ or $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 }$ is 1), $\begin{array} { r } { - \ \mathrm { i f } \ 1 \leq \sum _ { j = 0 } ^ { 2 } { \vec { b } } _ { i - 1 } ^ { j } \leq 2 , { \vec { p } } _ { i } = 1 \ \mathrm { ( i . e . } } \end{array}$ , the probability $2 ^ { - { \vec { p } } _ { i } }$ of $\vec { b } _ { i } ^ { 0 } = 1 \ \mathrm { o r } \ \vec { b } _ { i } ^ { 0 } = 0 \ \mathrm { i s } \ \frac { 1 } { 2 } )$ . 

We remark that $\Psi _ { 0 }$ and $\Psi _ { 1 }$ can be simplified by ${ \vec { c } } _ { 0 } = 0$ , and the semantic rule $\textstyle { \mathbb { I } } x - y { \mathbb { I } } _ { \gamma } ^ { \mathsf { E B } }$ is defined the same as $\mathbb { I } ^ { \boldsymbol { x } + \boldsymbol { y } } \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathsf { E B } }$ because $z = x - y$ iff $x = z + y$ . 

• The semantic rule $\mathbb { I } { x } \langle y \rangle \mathbb { I } _ { \gamma } ^ { \mathsf { E B } }$ follows the result given in Section 4.3, namely, $( \vec { b } , \vec { b } ^ { \prime } , p _ { i _ { 1 } } , \cdot \cdot \cdot , p _ { i _ { k } } )$ is a solution of $\Psi _ { S } ^ { \dagger }$ iff the probability $\mathsf { P r } _ { S } ( \vec { b } , \vec { b } ^ { \prime } )$ is $2 ^ { - \sum _ { j = 1 } ^ { k } t _ { i _ { j } } \cdot p _ { i _ { j } } }$ . Note that $t _ { i _ { j } }$ ’s are constants and $\Psi _ { S } ^ { \flat \ n }$ is added to boost MILP solving. 

Lemma 7.1. Suppose $\mathbb { [ } e \ b ] \ b { \mathscr { r } } _ { \gamma } ^ { \mathsf { E B } } = ( \Psi , \vec { b } , \varrho )$ with $\Psi \ne \emptyset$ . The assignment $\{ b _ { 1 } , \cdot \cdot \cdot , b _ { m } , p _ { 1 } , \cdot \cdot \cdot , p _ { n } \}$ is a solution $o f \Psi$ iff the proba $b i l i t y o f \{ b _ { 1 } , \cdot \cdot \cdot , b _ { i } \}$ being bit-level differences of the operands and result of ?? is $2 ^ { - \varrho [ { p } _ { 1 } , \cdots , { p } _ { n } ] }$ , where $\varrho [ p _ { 1 } , \cdots , p _ { n } ]$ denotes the value of ?? under the assignment $\{ p _ { 1 } , \cdots , p _ { n } \}$ of the Boolean variables for encoding probabilities, and $\{ b _ { i + 1 } , \cdots , b _ { m } \}$ is the assignment of the auxiliary Boolean variables if exist. 

## 7.2 Extended Bit-wise Differential Denotational Semantics for Statements

The (extended bit-wise differential) denotational semantics of a statement ?? is given by $\mathbb { [ } S \mathbb { ] } ^ { \mathtt { E B } }$ that maps each state $\gamma \in \Gamma$ to a triple $( \Psi , \gamma ^ { \prime } , \varrho )$ , denoted by $\mathbb { I } ^ { S } \mathbb { I } _ { \gamma } ^ { \mathsf { E B } }$ , where $\Psi , \gamma ^ { \prime }$ and $\varrho$ are the same as above. The extended bit-wise differential semantic rules for statements in EasyBC are similar to those word-wise ones, where $\left[ \left[ S \right] \right] _ { \gamma } ^ { \mathsf { E B } } = \left( \Psi , \gamma ^ { \prime } , 0 \right) \mathrm { i f } \left[ \left[ S \right] \right] _ { \gamma } ^ { \mathsf { B } } = \left( \Psi , \gamma ^ { \prime } , \theta \right)$ , except for 

$$
\begin{array}{c} \frac {\llbracket e \rrbracket_ {\gamma} ^ {\mathsf {E B}} = (\Psi , \vec {b} , \varrho)}{\llbracket x = e \rrbracket_ {\gamma} ^ {\mathsf {E B}} = (\Psi , \gamma [ x / \vec {b} ] , \varrho)} \\ \tau_ {0} f (\tau_ {1} x _ {1}, \dots , \tau_ {m} x _ {m}) \{S _ {1}; \dots ; S _ {n}; \texttt {r e t u r n} y; \} \quad \gamma_ {0} = \gamma [ x _ {1} / \gamma (y _ {1}) ] \dots [ x _ {m} / \gamma (y _ {m}) ] \\ \llbracket S _ {1} \rrbracket_ {\gamma_ {0}} ^ {\mathsf {E B}} = (\Psi_ {1}, \gamma_ {1}, \varrho_ {1}), \dots , \llbracket S _ {n} \rrbracket_ {\gamma_ {n - 1}} ^ {\mathsf {E B}} = (\Psi_ {n}, \gamma_ {n}, \varrho_ {n}) \\ \hline \llbracket x = g (y _ {1}, \dots , y _ {m}) \rrbracket_ {\gamma} ^ {\mathsf {E B}} = (\bigcup_ {i = 1} ^ {n} \Psi_ {i}, \gamma [ x / \gamma_ {n} (y) ], \sum_ {i = 1} ^ {n} \varrho_ {i}) \end{array}
$$

Intuitively, the semantic rule $\left[ [ x = g ( y _ { 1 } , \cdot \cdot \cdot , y _ { m } ) ] \right] _ { Y } ^ { \mathsf { E B } }$ sums up the expressions $\varrho _ { i } { } ^ { \mathbf { \ ' } } \mathbf { s }$ of the statements $\boldsymbol { S _ { i } } ^ { \prime } \boldsymbol { s }$ in the function body because of $\begin{array} { r } { \prod _ { i = 1 } ^ { n } 2 ^ { - \varrho _ { i } } = 2 ^ { - \sum _ { i = 1 } ^ { n } \varrho _ { i } } } \end{array}$ . 

## 7.3 Extended Bit-wise Resistance Evaluation

To evaluate the resistance of the program ?? in an extended bit-wise manner, we define the (extended bit-wise) semantics $\mathbb { [ } P \mathbb { ] } ^ { \mathtt { E B } }$ of the program ?? using its fn function ?? as follows: 

$$
\llbracket P \rrbracket^ {\mathsf {E B}} = \llbracket \text {   uints   } [ n ] f (\text {   uints   } [ n _ {1} ] k, \text {   uints   } [ n ] t x t) \{S _ {1}; \dots ; S _ {n}; \text {   return   } y; \} \rrbracket^ {\mathsf {E B}} = (\Psi , \gamma_ {n}, \varrho)
$$

where $\begin{array} { r } { \Psi = \bigcup _ { i = 1 } ^ { n } \Psi _ { i } , \varrho = \sum _ { i = 1 } ^ { n } \varrho _ { i } , \big [ S _ { 1 } \big ] \big | _ { \gamma _ { 0 } } ^ { \mathtt { b } } = \big ( \Psi _ { 1 } , \gamma _ { 1 } , \varrho _ { 1 } \big ) , \cdots , \big [ \big [ S _ { n } \big ] \big | _ { \gamma _ { n - 1 } } ^ { \mathtt { b } } = \big ( \Psi _ { n } , \gamma _ { n } , \varrho _ { n } \big ) \big ] } \end{array}$ and $\gamma _ { 0 }$ is an initial state mapping each bit of array elements of ?????? to a fresh Boolean variable. We get that: 

Theorem 7.2. Let $\ [ \boldsymbol { P } ] ^ { \mathsf { E B } } = ( \Phi , \gamma , \varrho )$ and ?? be the minimum value of ?? subject to the set of IL constraints Φ $\cup \left\{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \right\}$ . The MaxEDCP of the program ?? is no greater than $2 ^ { - u }$ . 

Similar to the decomposition approach given in Proposition 5.3, we have 

Proposition 7.3. Let $u _ { 1 }$ and $u _ { 2 }$ be the MaxEDCP of the first $s _ { 1 }$ -round and the subsequent $s _ { 2 } -$ round differential characteristics. Then, $u _ { 1 } \cdot u _ { 2 }$ is an upper bound of the MaxEDCP $o f ( s _ { 1 } + s _ { 2 } )$ -round differential characteristics. 

By Theorem 7.2 and Proposition 7.3, we have the following corollaries. 

Corollary 7.4. Let $\begin{array} { r } { [ P ] ^ { \sharp \mathsf { B } } = ( \Phi , \gamma , \varrho ) . I f \{ \varrho < \ell \} \cup \Phi \cup \{ \big ( \sum _ { i = 0 } ^ { n - 1 } \gamma \big ( t x t , i \big ) \big ) \ge 1 \} } \end{array}$ is unsatisfiable, then the program ?? with block size ?? is resistant against differential cryptanalysis. 


Table 3. Statistics of NIST candidates.


<table><tr><td rowspan="2">Name</td><td colspan="3">EASYBC</td><td colspan="2">C/C++</td></tr><tr><td><eq>LOC_{bw}</eq></td><td><eq>LOC_{ww}</eq></td><td>Time</td><td>LOC</td><td>Time</td></tr><tr><td>ASCON(<eq>\rho^{a}</eq>) [Dobraunig et al. 2016]♦</td><td>55</td><td>N/A</td><td>27.7</td><td>42</td><td>1.4</td></tr><tr><td>ELEPHANT [Beyne et al. 2020]♦</td><td>37</td><td>N/A</td><td>65.3</td><td>69</td><td>6.6</td></tr><tr><td>GIFT-COFB-128 [Banik et al. 2020]</td><td>34</td><td>N/A</td><td>23.9</td><td>81</td><td>3.0</td></tr><tr><td>GRAIN(AEAD) [Hell et al. 2021]*</td><td>99</td><td>N/A</td><td>0.1</td><td>146</td><td>0.1</td></tr><tr><td>ISAP(v2.0) [Dobraunig et al. 2020]♦</td><td>72</td><td>N/A</td><td>141.8</td><td>94</td><td>0.0</td></tr><tr><td>PHOTON(Beetle) [Bao et al. 2019]♦</td><td>-</td><td>51</td><td>24.4</td><td>104</td><td>1.8</td></tr><tr><td>ROMULUS-128 [Iwata et al. 2020]</td><td>57</td><td>N/A</td><td>32.3</td><td>113</td><td>0.2</td></tr><tr><td>SPARKLE256(slim) [Beierle et al. 2019]♦</td><td>39</td><td>N/A</td><td>0.3</td><td>19</td><td>0.0</td></tr><tr><td>TinyJAMBU [Wu and Huang 2019]♦</td><td>15</td><td>N/A</td><td>143.3</td><td>18</td><td>0.5</td></tr><tr><td>XOODYAK [Daemen et al. 2020]♦</td><td>38</td><td>N/A</td><td>21.1</td><td>63</td><td>0.1</td></tr></table>


Time is in milliseconds. N/A: word-wise is not applicable. -?? gives the block size required for security evaluation of block ciphers. $\mathrm { L O C } _ { \mathsf { b w / L O C } _ { \mathsf { w w } } \colon }$ LOC of bit-/word-wise implementation. ♦ indicates key-less permutations and  indicates stream cipher where no block size is given. 



Table 4. Statistics of other block ciphers.


<table><tr><td rowspan="2">Name</td><td colspan="3">EASYBC</td><td colspan="2">C/C++</td></tr><tr><td><eq>LOC_{bw}</eq></td><td><eq>LOC_{ww}</eq></td><td>Time</td><td>LOC</td><td>Time</td></tr><tr><td>AES-128 [Daemen and Rijmen 1999]</td><td>-</td><td>71</td><td>9.0</td><td>106</td><td>12.5</td></tr><tr><td>DES-64 [Fox 2000]</td><td>50</td><td>N/A</td><td>4.0</td><td>77</td><td>2.4</td></tr><tr><td>GIFT-64 [Banik et al. 2017]</td><td>34</td><td>N/A</td><td>6.5</td><td>63</td><td>1.9</td></tr><tr><td>KLEIN-64 [Gong et al. 2011]</td><td>-</td><td>64</td><td>8.1</td><td>97</td><td>0.5</td></tr><tr><td>LBLOCK-64 [Wu and Zhang 2011]</td><td>-</td><td>44</td><td>6.0</td><td>78</td><td>1.7</td></tr><tr><td>MIBS-64 [Izadi et al. 2009]</td><td>-</td><td>37</td><td>13.9</td><td>69</td><td>0.3</td></tr><tr><td>PICCOLO-64 [Shibutani et al. 2011]</td><td>-</td><td>85</td><td>7.4</td><td>90</td><td>84.5</td></tr><tr><td>PRESENT-64 [Bogdanov et al. 2007]</td><td>28</td><td>N/A</td><td>8.1</td><td>87</td><td>0.6</td></tr><tr><td>RECTANGLE-64 [Zhang et al. 2015]</td><td>28</td><td>N/A</td><td>6.6</td><td>80</td><td>1.6</td></tr><tr><td>SIMON-32 [Beaulieu et al. 2015]</td><td>35</td><td>N/A</td><td>3.0</td><td>46</td><td>0.8</td></tr><tr><td>SIMON-48 [Beaulieu et al. 2015]</td><td>35</td><td>N/A</td><td>5.9</td><td>46</td><td>7.1</td></tr><tr><td>SKINNY-64 [Beierle et al. 2016]</td><td>38</td><td>N/A</td><td>10.7</td><td>67</td><td>0.3</td></tr><tr><td>TWINE-64 [Suzaki et al. 2012]</td><td>-</td><td>43</td><td>9.8</td><td>61</td><td>4.4</td></tr></table>

Corollary 7.5. If the ??-round cipher ?? can be partitioned into $\frac { s } { s ^ { \prime } }$ identical $s ^ { \prime } .$ -round ciphers $P ^ { \prime }$ such that $\bigl \ [ P ^ { \prime } \bigr ] \bigr ^ { \mathsf { E B } } = ( \Phi , \gamma , \varrho )$ and $\begin{array} { r } { \{ \varrho < \frac { s ^ { \prime } \cdot \mathcal { Q } } { s } \} \cup \Phi \cup \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq \bar { 1 } \} } \end{array}$ is unsatisfiable, then the program ?? with block size ?? is resistant against differential cryptanalysis. 

## 8 EVALUATION

Our approach is implemented as an open-source tool. As shown in Figure 5, it utilizes the SMT solver Z3 for computing the branch number and solving MaxSMT problems and Gurobi [Gurobi Optimization 2018] for solving MILP. In general, EasyBC iteratively increases the round number from 1 (cf. Corollary 5.4 and Corollary 7.4). It also partitions an ??-round cipher to the maximum number of identical $s ^ { \prime } { \mathrm { - r o u n d } }$ ciphers and iteratively increases the round number $s ^ { \prime }$ (cf. Corollary 5.5 and Corollary 7.5), until the resistance is proved. The tool is designed to be modular and extensible, where each semantic rule has an API wrapper and alternative generation methods can be easily chosen and added. We also incorporate the bounding condition [Matsui 1994; Zhang et al. 2018] into MILP to prune search space which often improves the overall MILP solving. (The detail is given in [Sun et al. 2023, Section E.4].) 

All the experiments were conducted on a machine with two Intel Xeon Gold 5118 CPUs (12 cores, 2.30GHz), 64-bit Ubuntu 20.04 LTS, and 128GB RAM. The number of threads for Gurobi is set to 16. 

## 8.1 Expressiveness of EasyBC

To evaluate the expressiveness of EasyBC, we implement 23 realistic cryptographic primitives with EasyBC, consisting of all the 10 finalists of the NIST lightweight cryptography standardization process [NIST 2023] and 13 commonly used block ciphers, covering both SPN ciphers (e.g., AES, PRESENT, and GIFT-COFB) and BFN ciphers (e.g., DES, LBLOCK, TWINE). We stress that we focus on the underlying primitives (i.e., block ciphers and key-less permutations) rather than the full authenticated encryption, message authentication, or hash protocols in the NIST finalists. 

The physical source lines of code (LOC [Nguyen et al. 2007]) counted by cloc [Danial 2021] are reported in Tables 3 and 4. We report LOC of word-wise (when available, or otherwise bit-wise) EasyBC implementations. As a comparison, we also report LOC of the C/C++ reference implementations of the NIST finalists and (randomly selected) GitHub open-source C/C++ implementations of other block ciphers. To some extent, a smaller number (highlighted in bold) indicates that the language is more succinct in implementing cryptographic primitives. 

We observe that EasyBC is sufficiently expressive to easily implement all the 23 cryptographic primitives either in word-wise or bit-wise fashion. More specifically, when cryptographic primitives can be implemented in a word-wise fashion (i.e., PHOTON, AES, KLEIN, LBLOCK, MIBS, 


Table 5. Results of the word-wise approach, where (??) indicates the number of entire rounds of the cipher and #AS denotes the lower bound of the minimum number of active S-boxes.


<table><tr><td colspan="2">Rounds</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td><td>16</td><td>17</td><td>18</td><td>19</td><td>20</td><td>21</td><td>22</td><td>23</td><td>24</td><td>25</td></tr><tr><td rowspan="2">AES (14)</td><td>#AS</td><td>1</td><td>5</td><td>9</td><td>25</td><td>26</td><td>30</td><td>34</td><td>50</td><td>51</td><td>55</td><td>59</td><td>75</td><td>76</td><td>80</td><td colspan="11">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td colspan="11">N/A</td></tr><tr><td rowspan="2">KLEIN (12)</td><td>#AS</td><td>1</td><td>5</td><td>8</td><td>15</td><td>16</td><td>20</td><td>23</td><td>30</td><td>31</td><td>35</td><td>38</td><td>45</td><td colspan="13">N/A</td></tr><tr><td>Time</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td colspan="13">N/A</td></tr><tr><td rowspan="2">LBLOCK (32)</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>6</td><td>8</td><td>11</td><td>14</td><td>18</td><td>22</td><td>24</td><td>27</td><td>30</td><td>32</td><td>35</td><td>36</td><td>39</td><td>41</td><td>44</td><td>45</td><td>48</td><td>50</td><td>53</td><td>54</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>1s</td><td>1s</td><td>3s</td><td>3s</td><td>2s</td><td>6s</td><td>5s</td><td>62s</td><td>62s</td><td>80s</td></tr><tr><td rowspan="2">MIBS (32)</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>5</td><td>6</td><td>7</td><td>8</td><td>11</td><td>12</td><td>13</td><td>14</td><td>17</td><td>18</td><td>19</td><td>20</td><td>23</td><td>24</td><td>25</td><td>26</td><td>29</td><td>30</td><td>31</td><td>32</td><td>35</td><td>36</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>1s</td><td>1s</td><td>1s</td><td>3s</td><td>4s</td><td>2s</td><td>4s</td><td>2s</td><td>3s</td><td>2s</td><td>3s</td><td>4s</td><td>5s</td><td>4s</td><td>7s</td></tr><tr><td rowspan="2">PHOTON (12)</td><td>#AS</td><td>1</td><td>9</td><td>17</td><td>81</td><td>82</td><td>90</td><td>98</td><td>162</td><td>163</td><td>171</td><td>179</td><td>243</td><td colspan="13">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td colspan="13">N/A</td></tr><tr><td rowspan="2">PICCOLO (25)</td><td>#AS</td><td>0</td><td>5</td><td>10</td><td>15</td><td>20</td><td>30</td><td>35</td><td>40</td><td>45</td><td>50</td><td>55</td><td>60</td><td>65</td><td>70</td><td>75</td><td>80</td><td>85</td><td>90</td><td>95</td><td>100</td><td>105</td><td>110</td><td>115</td><td>120</td><td>125</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>3s</td><td>6s</td><td>4s</td><td>6s</td><td>28s</td><td>30s</td><td>31s</td><td>38s</td><td>187s</td><td>93s</td><td>122s</td><td>289s</td><td>560s</td></tr><tr><td rowspan="2">TWINE (36)</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>6</td><td>8</td><td>11</td><td>14</td><td>18</td><td>22</td><td>24</td><td>27</td><td>30</td><td>32</td><td>35</td><td>36</td><td>39</td><td>41</td><td>44</td><td>45</td><td>48</td><td>50</td><td>53</td><td>54</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>2s</td><td>2s</td><td>3s</td><td>2s</td><td>5s</td><td>6s</td><td>26s</td><td>79s</td><td>72s</td></tr></table>

PICCOLO, and TWINE), their EasyBC implementations always require considerably less code than the baselines. The main reason is that EasyBC provides high-level constructs of P-box and matrix-vector products for permutations and linear transformations. For cryptographic primitives that are implemented in a bit-wise fashion with EasyBC, their EasyBC implementations also require significantly less code than their baselines except for ASCON and SPARKLE, due to the following reasons. (1) One 320-bit block is stored in five 64-bit variables in the reference implementation of ASCON each of which is permuted, while one 320-bit block is stored in one Boolean array in the EasyBC implementation so that the 320-bit Boolean array has to be split before the permutation and merged after permutation. (2) The C++ reference implementation of SPARKLE uses function-like macro definitions and thus is more succinct. 

Results of EasyBC interpreter. For each realistic cryptographic primitive, we randomly generate 100 inputs to EasyBC programs and binary executables of C/C++ programs. We run the EasyBC program (using our interpreter) and the binary executable for each input and record the output and the execution time. The outputs of the respective programs have been compared with 100% match, which validates the semantics and the interpreter of EasyBC, as well as EasyBC programs. The average execution times over 100 inputs are reported in Tables 3 and 4. While executing via our interpreter is less efficient, it is acceptable for testing EasyBC programs. 

## 8.2 Effectiveness of EasyBC

To evaluate the effectiveness of EasyBC, we first compare the performance of various alternative methods to generate MILP. According to our experiment results (cf. [Sun et al. 2023, Section E]), $\Psi _ { 2 , 3 } ^ { 1 }$ $\Psi _ { M } ^ { 1 }$ are used for modeling the modular addition, substitution, XOR and matrix-vector product respectively in the $\psi _ { \oplus } ^ { 2 } , \Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 2 }$ $\Psi _ { S } ^ { 3 }$ in the bit-wise approach (cf. Figure 8 and Section 4.2); the technique of [Sasaki and Todo 2017] is used for constructing $\Psi _ { S } ^ { 4 }$ in the extended bit-wise approach (cf. Section 4.3). We remark that there is no consensus on the security of key-less permutations yet, and they are used to evaluate the performance of EasyBC instead of security evaluation. 

8.2.1 Word-wise Approach. The word-wise approach is evaluated on all the word-wise implementations. The results are reported in Table 5 up to 25 rounds which suffice to prove security. The results for the entire rounds are given in [Sun et al. 2023, Section E.1]. 


Table 6. Results of the bit-wise approach with the bounding condition, where Timeout is 24 hours.


<table><tr><td colspan="2">Rounds</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td rowspan="2">ASCON (12)</td><td>#AS</td><td>1</td><td>4</td><td>15</td><td colspan="12">N/A</td></tr><tr><td>Time</td><td>0s</td><td>1061s</td><td>1573s</td><td colspan="12">Timeout</td></tr><tr><td rowspan="2">DES (16)</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>4</td><td>6</td><td>8</td><td>9</td><td>12</td><td>12</td><td colspan="6">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>6s</td><td>61s</td><td>355s</td><td>1073s</td><td>27294s</td><td>57344s</td><td colspan="6">Timeout</td></tr><tr><td rowspan="2">ELEPHANT (80)</td><td>#AS</td><td>1</td><td>2</td><td>4</td><td>6</td><td>10</td><td>12</td><td>14</td><td>16</td><td>18</td><td>20</td><td>22</td><td>24</td><td colspan="3">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>4s</td><td>5s</td><td>50s</td><td>394s</td><td>1653s</td><td>1592s</td><td>2994s</td><td>4353s</td><td>18294s</td><td>20404s</td><td colspan="3">Timeout</td></tr><tr><td rowspan="2">GIFT-COFB (40)</td><td>#AS</td><td>1</td><td>2</td><td>3</td><td>5</td><td>7</td><td>10</td><td>13</td><td>17</td><td>19</td><td>21</td><td colspan="5">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>7s</td><td>7s</td><td>38s</td><td>775s</td><td>2222s</td><td>6725s</td><td>77870s</td><td colspan="5">Timeout</td></tr><tr><td rowspan="2">GIFT (28)</td><td>#AS</td><td>1</td><td>2</td><td>3</td><td>5</td><td>7</td><td>10</td><td>13</td><td>16</td><td>18</td><td>20</td><td>22</td><td>24</td><td>26</td><td colspan="2">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>1s</td><td>2s</td><td>3s</td><td>7s</td><td>52s</td><td>43s</td><td>136s</td><td>518s</td><td>846s</td><td>25561s</td><td colspan="2">Timeout</td></tr><tr><td rowspan="2">PRESENT (31)</td><td>#AS</td><td>1</td><td>2</td><td>4</td><td>6</td><td>10</td><td>12</td><td>14</td><td>16</td><td>18</td><td>20</td><td>22</td><td>24</td><td>26</td><td>28</td><td>N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>6s</td><td>8s</td><td>15s</td><td>65s</td><td>95s</td><td>539s</td><td>1884s</td><td>10271s</td><td>38907s</td><td>50931s</td><td>Timeout</td></tr><tr><td rowspan="2">RECTANGLE (25)</td><td>#AS</td><td>1</td><td>2</td><td>3</td><td>4</td><td>6</td><td>8</td><td>11</td><td>13</td><td>15</td><td>17</td><td>19</td><td>21</td><td>23</td><td>25</td><td>27</td></tr><tr><td>Time</td><td>1s</td><td>0s</td><td>0s</td><td>1s</td><td>8s</td><td>21s</td><td>6s</td><td>436s</td><td>63s</td><td>603s</td><td>1135s</td><td>1221s</td><td>2841s</td><td>36333s</td><td>37860s</td></tr><tr><td rowspan="2">ROMULUS (40)</td><td>#AS</td><td>1</td><td>2</td><td>5</td><td>8</td><td>12</td><td>16</td><td>26</td><td>36</td><td>41</td><td colspan="6">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>5s</td><td>48s</td><td>87s</td><td>378s</td><td>1526s</td><td>17266s</td><td>4043s</td><td colspan="6">Timeout</td></tr><tr><td rowspan="2">SKINNY (36)</td><td>#AS</td><td>1</td><td>2</td><td>5</td><td>8</td><td>12</td><td>16</td><td>26</td><td>36</td><td>41</td><td>46</td><td>51</td><td>55</td><td colspan="3">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>2s</td><td>15s</td><td>24s</td><td>78s</td><td>967s</td><td>2041s</td><td>3610s</td><td>15502s</td><td>26989s</td><td colspan="3">Timeout</td></tr><tr><td rowspan="2">SPARKLE (7)</td><td>#AS</td><td>1</td><td>2</td><td>5</td><td>6</td><td>7</td><td colspan="10">N/A</td></tr><tr><td>Time</td><td>2s</td><td>44s</td><td>1624s</td><td>4416s</td><td>17592s</td><td colspan="10">Timeout</td></tr></table>

We observe that the execution time (i.e., the MILP solving time) in seconds (s) increases with the round number. The execution time of PICCOLO is considerably higher than that of the others when the round number is large $( \mathbf { e . g . } , \geq 1 3 )$ , because PICCOLO generates more constraints. For instance, the number of constraints for the 20-round LBLOCK and TWINE are both 481, while it is 641 for the 20-round PICCOLO. However, the large round number is not always necessary, as the security may have been proved with a small round number (see below). 

For block ciphers, $p = 2 ^ { - 6 }$ and $\mathcal { l } = 1 2 8$ for AES; $\ p = 2 ^ { - 2 }$ and $\mathcal { Q } = 6 4$ for KLEIN, LBLOCK, MIBS, PICCOLO and TWINE; $\ p = 2 ^ { - 2 }$ and $\mathcal { k } = 1 2 8$ for PHOTON, where the maximum probability ?? of the involved S-boxes that are arrays is computed by enumeration. By Corollary 5.4, EasyBC proved that AES (resp. KLEIN, LBLOCK, MIBS, PHOTON, PICCOLO and TWINE) is resistant when the round number ?? is 4 (resp. 10, 15, 23, 4, 7 and 15), as highlighted in boldface in Table 5. 

8.2.2 Bit-wise Approach. The bit-wise approach is evaluated on all bit-wise implementations with S-boxes. (The word-wise implementations are excluded.) The results are given in Table 6 up to 15 rounds within 24 hours, where the execution time is the MILP solving time. 

Unsurprisingly, we observe that the execution time increases very quickly with the round number due to the blow-up of constraints. For instance, the numbers of constraints and involved variables of ASCON are 6,913 and 1,408 respectively for 1 round, but become 13,825 and 2,496 (resp. 20,737 and 3,584) for 2 (resp. 3) rounds. 

For block ciphers, $\hbar = 2 ^ { - 2 }$ and $\mathcal { Q } = 6 4$ for DES, PRESENT, RECTANGLE and SKINNY; $p = 2 ^ { - 1 . 4 1 5 }$ and $\mathcal { Q } \ : = \ : 1 2 8$ for GIFT-COFB; $p = 2 ^ { - 1 . 4 1 5 }$ and $\mathcal { U } = 6 4$ for GIFT; and $p = 2 ^ { - 2 }$ and $\mathcal { l } = 1 2 8$ for ROMULUS. We found that by Corollary 5.4, EasyBC cannot prove that DES (resp. GIFT-COFB, GIFT, PRESENT, RECTANGLE, ROMULUS and SKINNY) is resistant using the lower bounds of numbers of active S-boxes reported in Table 6. Fortunately, by Corollary 5.5, EasyBC proved that GIFT (resp. PRESENT, RECTANGLE, ROMULUS and SKINNY) is resistant with 6 (resp. 3, 6, 3 and 1) rounds, as highlighted in boldface in Table 6. Note that the resistance of GIFT-COFB cannot be proved here which will be done by applying the extended bit-wise approach later, while DES is indeed vulnerable to differential cryptanalysis [Biham and Shamir 1990] and EasyBC can return the differential characteristics up to 9 rounds within 24-hour time limit. 

We also compared the MILP solving time with/without bounding conditions. Adding the bounding condition often (5 out of 8) improves the efficiency by 1 to 3 times, but does not necessarily improve (and sometimes even worsens) the efficiency (3 out of 8). The results without the bounding condition are given in [Sun et al. 2023, Section E.5]. 


Table 7. Results of the extended bit-wise approach with the bounding condition, where Pr denotes the upper bound of the probability of optimal differential characteristics, and Timeout is 24 hours.


<table><tr><td colspan="2">Rounds r</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td rowspan="2">GIFT-COFB (40)</td><td>Pr</td><td><eq>2^{-1.415}</eq></td><td><eq>2^{-3.415}</eq></td><td><eq>2^{-7}</eq></td><td><eq>2^{-11.415}</eq></td><td><eq>2^{-17}</eq></td><td><eq>2^{-22.415}</eq></td><td><eq>2^{-28.415}</eq></td><td colspan="8">N/A</td></tr><tr><td>Time</td><td>0s</td><td>6s</td><td>9s</td><td>124s</td><td>1297s</td><td>5811s</td><td>11538s</td><td colspan="8">Timeout</td></tr><tr><td rowspan="2">SIMON-32 (32)</td><td>Pr</td><td><eq>2^{0}</eq></td><td><eq>2^{-2}</eq></td><td><eq>2^{-4}</eq></td><td><eq>2^{-6}</eq></td><td><eq>2^{-8}</eq></td><td><eq>2^{-12}</eq></td><td><eq>2^{-14}</eq></td><td><eq>2^{-18}</eq></td><td><eq>2^{-20}</eq></td><td><eq>2^{-26}</eq></td><td><eq>2^{-30}</eq></td><td><eq>2^{-34}</eq></td><td><eq>2^{-36}</eq></td><td><eq>2^{-38}</eq></td><td><eq>2^{-40}</eq></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>4s</td><td>3s</td><td>7s</td><td>84s</td><td>57s</td><td>820s</td><td>191s</td><td>6327s</td></tr><tr><td rowspan="2">SIMON-48 (36)</td><td>Pr</td><td><eq>2^{0}</eq></td><td><eq>2^{-2}</eq></td><td><eq>2^{-4}</eq></td><td><eq>2^{-6}</eq></td><td><eq>2^{-8}</eq></td><td><eq>2^{-12}</eq></td><td><eq>2^{-14}</eq></td><td><eq>2^{-18}</eq></td><td><eq>2^{-20}</eq></td><td><eq>2^{-26}</eq></td><td><eq>2^{-30}</eq></td><td><eq>2^{-36}</eq></td><td><eq>2^{-38}</eq></td><td><eq>2^{-44}</eq></td><td><eq>2^{-46}</eq></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>1s</td><td>3s</td><td>5s</td><td>14s</td><td>16s</td><td>101s</td><td>95s</td><td>402s</td><td>505s</td><td>53920s</td></tr><tr><td rowspan="2">Alzette (12)</td><td>Pr</td><td><eq>2^{0}</eq></td><td><eq>2^{-1}</eq></td><td><eq>2^{-2}</eq></td><td><eq>2^{-6}</eq></td><td><eq>2^{-10}</eq></td><td><eq>2^{-18}</eq></td><td colspan="9">N/A</td></tr><tr><td>Time</td><td>0s</td><td>1s</td><td>2s</td><td>75s</td><td>221s</td><td>3897s</td><td colspan="9">Timeout</td></tr></table>

8.2.3 Extended Bit-wise Approach. The capability of the extended bit-wise approach is evaluated on all the bit-wise implementations of block ciphers and the S-box of SPARKLE (i.e., 4-round Alzette) that cannot be proved or analyzed before. The results are given in Table 7 up to 15 rounds within 24 hours, where the execution time is the MILP solving time. 

Unsurprisingly, the execution time of the extended bit-wise approach is longer than that of the bit-wise approach. For instance, on the 7-round GIFT-COFB, the execution time of the extended bit-wise approach is 11,538s while the execution time of the bit-wise approach is 1,074s. This is because probabilities are explicitly encoded using additional Boolean variables in the extended bit-wise approach, resulting in more difficult MILP instances. 

Note the block size: GIFT-COFB ?? = 128, SIMON-32 ?? = 32, and SIMON-48 $\mathcal { k } \ = \ 4 8 .$ . By Corollary 7.5, EasyBC prove that GIFT-COFB (resp. SIMON-32 and SIMON-48) is resistant with 5 (resp. 2 and 3) rounds. 

## 9 RELATED WORK

[Matsui 1994] proposed the first algorithm for automated resistance analysis of block ciphers against differential cryptanalysis. To further enhance efficiency, various heuristics have been proposed to reduce the search space [Aoki et al. 1997; Bao et al. 2014; Biryukov and Nikolić 2010; Ji et al. 2021]. However, these heuristics generally rely on cipher-specific optimizations, necessitating sophisticated programming skills. Additionally, creating highly reusable code that can be easily adapted for different ciphers is non-trivial [Zhang et al. 2018]. 

In recent years, a more promising approach based on MILP has been developed. [Mouha et al. 2011] proposed to determine the lower bound of the minimum number of active S-boxes via MILP solving. As an early attempt, it only considered the word-wise modeling for the XOR operation, S-box, and linear transformation. To partially lift this limitation, [Sun et al. 2013] introduced the bitwise modeling. To precisely characterize S-boxes in MILP, [Sun et al. 2014a] proposed to construct IL constraints from the H-representation of the S-box DDT and reduced the number of constraints via a greedy algorithm. Later, Sun et al. [Sun et al. 2014b] proposed a bit-wise modeling method for the AND operation and extended the method of [Sun et al. 2014a] to directly bound the MaxEDCP by encoding the probabilities between input and output differences of S-boxes in IL constraints. 

Since then, plenty of modeling methods for specific operations have been proposed, aimed at improving efficiency and applicability. [Sasaki and Todo 2017] proposed an MILP-based algorithm to reduce the number of constraints obtained from the H-representation of the S-box DDT. [Abdelkhalek et al. 2017] proposed to construct IL constraints from the minimized product-of-sum representation of S-box DDT instead of the H-representation. Along this line, [Boura and Coggia 2020; Li and Sun 2022; Sun 2021; Udovenko 2021] generate more diverse constraints from the S-box DDT, allowing the number of the resulting constraints to be further reduced by applying greedy or MILP-based algorithms; [Cui et al. 2016; Li et al. 2019; Sasaki and Todo 2017; Yin et al. 2017] proposed a new modeling for the XOR operation; [Boura and Coggia 2020; Ilter and Selçuk 2021; 

Zhang and Zhang 2018] proposed another modeling for linear transformation; [Fu et al. 2016] proposed a modeling method for the modular addition operation; and [Chen et al. 2015; Liu et al. 2017; Wang et al. 2018] proposed a modeling method for the rotation-AND operation. Besides new modeling methods for specific operations, optimization strategies have also been proposed. [Zhang et al. 2018] incorporated the bounding condition of [Matsui 1994] into the MILP-based method; [Zhou et al. 2019] proposed partitioning all possible differential characteristics into subsets, each of which is analyzed by the MILP-based method. 

Another direction is to resort to SAT/SMT solving. [Mouha and Preneel 2013] proposed the first SAT/SMT modeling for the XOR, modular addition, and rotation operations. It has been extended to handle a specific permutation [Aumasson et al. 2014], the AND operation and rotation with constants [Kölbl et al. 2015], independent modular addition [Song et al. 2016], S-boxes [Liu et al. 2021; Sun et al. 2018], and modular addition with constants [Azimi et al. 2022]. In contrast to the MILP-based method which determines the lower bound of the minimum number of active S-boxes or the upper bound of MaxEDCP, SAT/SMT-based methods can only verify whether a given number is a bound. Recently, both the bounding condition of [Matsui 1994] and MILP-based method have been combined with the SAT/SMT-based method [Makarim and Rohit 2022; Sun et al. 2021] to improve the efficiency. To facilitate the comparison of all the above MILP/SAT/SMT-based approaches, a summary is given in [Sun et al. 2023, Section F]. 

Despite significant progress in the field, existing works primarily concentrate on ad-hoc modeling methods designed for specific operations, but do not offer systematic methods for determining word-wise or bit-wise branch numbers and encoding probabilities between input and output differences. There is a lack of language support, unified computational approaches and full automation. This limitation forces cryptanalysts to individually model each cipher within the tool or create a model generation script for every individual cipher, resulting in a complex, error-prone, and time-consuming process. This work fills this significant gap and makes resistance evaluation against differential cryptanalysis easily accessible to cryptographers. 

There are other cryptography-specific languages such as SAW [Carter et al. 2013], Jasmin [Almeida et al. 2017], Vale [Bond et al. 2017], Usuba [Mercadier and Dagand 2019], FaCT [Cauligi et al. 2019], QMVerif [Gao et al. 2022], HOME [Gao et al. 2021], and FISCHER [Liu et al. 2023]. They are designed to ensure functional correctness and/or side-channel security of cryptographic algorithms or implementations, which are considerably different from EasyBC. 

## 10 CONCLUSION

We have designed a high-level cryptography-specific language EasyBC for describing block ciphers and presented a rigorous MILP generation procedure from EasyBC programs in the form of differential denotational semantics, leading to a generic and extensible approach for automatically evaluating the resistance of block ciphers written in EasyBC against differential cryptanalysis. We have implemented our approach in an open-source tool and extensively evaluate it on a set of realistic cryptographic primitives, demonstrating its expressivity and capability. In particular, experimental results show that realistic cryptographic primitives can be easily described in EasyBC and their resistance against differential cryptanalysis can be efficiently and effectively proved using our tool. Our tool enables cryptanalysts to easily assess the resistance of block ciphers against differential cryptanalysis in a fully automatic way. 

For future research, it would be interesting to improve efficiency by combining recent optimization strategies and to develop analysis approaches for other powerful cryptanalysis (e.g., linear cryptanalysis [Biryukov and De Cannière 2011], impossible differential cryptanalysis [Kim et al. 2003]) based on EasyBC. 

## ACKNOWLEDGEMENT

We thank the reviewers of POPL’24 for their constructive and insightful comments. This work is supported by the National Natural Science Foundation of China (NSFC) under Grants No. 62072309 and No. 61872340, CAS Project for Young Scientists in Basic Research (YSBR-040), ISCAS New Cultivation Project (ISCAS-PYFX-202201), overseas grants from the State Key Laboratory of Novel Software Technology, Nanjing University (KFKT2023A04), and Birkbeck BEI School Project (EF-FECT). 



Alex Biryukov and Ivica Nikolić. 2010. Automatic search for related-key differential characteristics in byte-oriented block ciphers: Application to AES, Camellia, Khazad and others. In Proceedings of the Annual International Conference on the Theory and Applications of Cryptographic Techniques. 322–344. 





Maryam Izadi, Babak Sadeghiyan, Seyed Saeed Sadeghian, and Hossein Arabnezhad Khanooki. 2009. MIBS: A New Lightweight Block Cipher. In Proceedings of the 8th International Conference on Cryptology and Network Security. 334–348. 





Ling Song, Zhangjie Huang, and Qianqian Yang. 2016. Automatic Differential Analysis of ARX Block Ciphers with Application to SPECK and LEA. In Proceedings of the 21st Australasian Conference on Information Security and Privacy. 379–394. 



## DATA AVAILABILITY STATEMENT



Nikolaj S. Bjørner and Anh-Dung Phan. 2014. ??Z - Maximal Satisfaction with Z3. In Proceedings of the 6th International Symposium on Symbolic Computation in Software Science. 1–9. 





Fulei Ji, Wentao Zhang, and Tianyou Ding. 2021. Improving matsui’s search algorithm for the best differential/linear trails and its applications for DES, DESL and GIFT. Comput. J. 64, 4 (2021), 610–627. 





Ling Sun, Wei Wang, and Meiqin Wang. 2018. More Accurate Differential Properties of LED64 and Midori64. IACR Trans. Symmetric Cryptol. 2018, 3 (2018), 93–123. 



The full version of the paper [Sun et al. 2023] contains missing proofs and more experimental results. Our tool is available at https://github.com/S3L-official/EasyBC. 



Nikolaj S. Bjørner, Anh-Dung Phan, and Lars Fleckenstein. 2015. ??Z - An Optimizing SMT Solver. In Proceedings of the 21st International Conference on Tools and Algorithms for the Construction and Analysis of Systems. 194–199. 





John B. Kam and George I. Davida. 1979. Structured design of substitution-permutation encryption networks. IEEE Trans. Comput. 28, 10 (1979), 747–753. 





Ling Sun, Wei Wang, and Meiqin Wang. 2021. Accelerating the Search of Differential and Linear Characteristics with the SAT Method. IACR Trans. Symmetric Cryptol. 2021, 1 (2021), 269–315. 



## REFERENCES



Andrey Bogdanov. 2010. Analysis and design of block cipher constructions. Ph. D. Dissertation. Ruhr University Bochum. 





Jonathan Katz and Yehuda Lindell. 2014. Introduction to Modern Cryptography, 2nd Edition. CRC Press. 





Pu Sun, Fu Song, Yuqi Chen, and Taolue Chen. 2023. EasyBC: A Cryptography-Specific Language for Security Analysis of Block Ciphers against Differential Cryptanalysis (Full version). Technical Report. https://github.com/S3L-official/EasyBC. 





Andrey Bogdanov, Lars R Knudsen, Gregor Leander, Christof Paar, Axel Poschmann, Matthew JB Robshaw, Yannick Seurin, and Charlotte Vikkelsoe. 2007. PRESENT: An ultra-lightweight block cipher. In Proceedings of the International workshop on cryptographic hardware and embedded systems. 450–466. 





Jongsung Kim, Seokhie Hong, Jaechul Sung, Sangjin Lee, Jongin Lim, and Soohak Sung. 2003. Impossible differential cryptanalysis for block cipher structures. In Proceedings of the International Conference on Cryptology in India. 82–96. 





Siwei Sun, Lei Hu, Ling Song, Yonghong Xie, and Peng Wang. 2013. Automatic security evaluation of block ciphers with S-bP structures against related-key differential attacks. In Proceedings of the International Conference on Information Security and Cryptology. 39–51. 





Ahmed Abdelkhalek, Yu Sasaki, Yosuke Todo, Mohamed Tolba, and Amr M Youssef. 2017. MILP modeling for (large) s-boxes to optimize probability of differential characteristics. IACR Transactions on Symmetric Cryptology (2017), 99–129. 





Barry Bond, Chris Hawblitzel, Manos Kapritsos, K. Rustan M. Leino, Jacob R. Lorch, Bryan Parno, Ashay Rane, Srinath T. V. Setty, and Laure Thompson. 2017. Vale: Verifying High-Performance Cryptographic Assembly Code. In Proceedings of the 26th USENIX Security Symposium, Engin Kirda and Thomas Ristenpart (Eds.). 917–934. 





Lars R Knudsen. 1998. Block Ciphers: a survey. In State of the art in applied cryptography. Springer, 18–48. 





Siwei Sun, Lei Hu, Meiqin Wang, Peng Wang, Kexin Qiao, Xiaoshuang Ma, Danping Shi, Ling Song, and Kai Fu. 2014b. Towards finding the best characteristics of some bit-oriented block ciphers and automatic enumeration of (related-key) differential and linear characteristics with predefined properties. Cryptology ePrint Archive (2014). 





José Bacelar Almeida, Manuel Barbosa, Gilles Barthe, Arthur Blot, Benjamin Grégoire, Vincent Laporte, Tiago Oliveira, Hugo Pacheco, Benedikt Schmidt, and Pierre-Yves Strub. 2017. Jasmin: High-Assurance and High-Speed Cryptography. In Proceedings of the 2017 ACM SIGSAC Conference on Computer and Communications Security. ACM, 1807–1823. https: //doi.org/10.1145/3133956.3134078 





Christina Boura and Daniel Coggia. 2020. Efficient MILP modelings for Sboxes and linear layers of SPN ciphers. IACR Transactions on Symmetric Cryptology (2020), 327–361. 





Stefan Kölbl, Gregor Leander, and Tyge Tiessen. 2015. Observations on the SIMON Block Cipher Family. In Proceedings of the 35th Annual Cryptology Conference. 161–185. 





Siwei Sun, Lei Hu, Peng Wang, Kexin Qiao, Xiaoshuang Ma, and Ling Song. 2014a. Automatic security evaluation and (related-key) differential characteristic search: application to SIMON, PRESENT, LBlock, DES(L) and other bit-oriented block ciphers. In Proceedings of the International Conference on the Theory and Application of Cryptology and Information Security. 158–178. 





Kazumaro Aoki, Kunio Kobayashi, and Shiho Moriai. 1997. Best differential characteristic search of FEAL. In Proceedings of the International Workshop on Fast Software Encryption. 41–53. 





Kyle Carter, Adam Foltzer, Joe Hendrix, Brian Huffman, and Aaron Tomb. 2013. SAW: the software analysis workbench. In Proceedings of the 2013 ACM SIGAda annual conference on High integrity language technology, Jeff Boleng and S. Tucker Taft (Eds.). ACM, 15–18. https://doi.org/10.1145/2527269.2527277 





Xuejia Lai and James L. Massey. 1990. A Proposal for a New Block Encryption Standard. In Proceedings of the Workshop on the Theory and Application of of Cryptographic Techniques (EUROCRYPT). 389–404. 





Yao Sun. 2021. Towards the Least Inequalities for Describing a Subset in $Z _ { 2 } \mathrm { n }$ . IACR Cryptol. ePrint Arch. (2021), 1084. 





Jean-Philippe Aumasson, Philipp Jovanovic, and Samuel Neves. 2014. Analysis of NORX: Investigating Differential and Rotational Properties. In Proceedings of the 3rd International Conference on Cryptology and Information Security in Latin America. 306–324. 





Sunjay Cauligi, Gary Soeller, Brian Johannesmeyer, Fraser Brown, Riad S. Wahby, John Renner, Benjamin Grégoire, Gilles Barthe, Ranjit Jhala, and Deian Stefan. 2019. FaCT: a DSL for timing-sensitive computation. In Proceedings of the 40th ACM SIGPLAN Conference on Programming Language Design and Implementation, Kathryn S. McKinley and Kathleen Fisher (Eds.). ACM, 174–189. https://doi.org/10.1145/3314221.3314605 





Xuejia Lai, James L Massey, and Sean Murphy. 1991. Markov ciphers and differential cryptanalysis. In Proceedings of the Workshop on the Theory and Application of of Cryptographic Techniques. 17–38. 





Tomoyasu Suzaki, Kazuhiko Minematsu, Sumio Morioka, and Eita Kobayashi. 2012. TWINE: A Lightweight Block Cipher for Multiple Platforms. In Proceedings of the International Conference on Selected Areas in Cryptography. 339–354. 





Seyyed Arash Azimi, Adrián Ranea, Mahmoud Salmasizadeh, Javad Mohajeri, Mohammad Reza Aref, and Vincent Rijmen. 2022. A bit-vector differential model for the modular addition by a constant and its applications to differential and impossible-differential cryptanalysis. Des. Codes Cryptogr. 90, 8 (2022), 1797–1855. 





Zhan Chen, Ning Wang, and Xiaoyun Wang. 2015. Impossible Differential Cryptanalysis of Reduced Round SIMON. IACR Cryptol. ePrint Arch. (2015), 286. 





Lingchen Li, Wenling Wu, Yafei Zheng, and Lei Zhang. 2019. The Relationship between the Construction and Solution of the MILP Models and Applications. IACR Cryptol. ePrint Arch. (2019), 49. 





Je Sen Teh and Alex Biryukov. 2022. Differential cryptanalysis of WARP. J. Inf. Secur. Appl. 70 (2022), 103316. 





Subhadeep Banik, Avik Chakraborti, Tetsu Iwata, Kazuhiko Minematsu, Mridul Nandi, Thomas Peyrin, Yu Sasaki, Siang Meng Sim, and Yosuke Todo. 2020. GIFT-COFB. IACR Cryptol. ePrint Arch. (2020), 738. 





Tingting Cui, Keting Jia, Kai Fu, Shiyao Chen, and Meiqin Wang. 2016. New Automatic Search Tool for Impossible Differentials and Zero-Correlation Linear Approximations. IACR Cryptol. ePrint Arch. (2016), 689. 





Ting Li and Yao Sun. 2022. SuperBall: A New Approach for MILP Modelings of Boolean Functions. IACR Transactions on Symmetric Cryptology (2022), 341–367. 





Aleksei Udovenko. 2021. MILP modeling of Boolean functions by minimum number of inequalities. Cryptology ePrint Archive (2021). 





Subhadeep Banik, Sumit Kumar Pandey, Thomas Peyrin, Yu Sasaki, Siang Meng Sim, and Yosuke Todo. 2017. GIFT: A Small Present - Towards Reaching the Limit of Lightweight Encryption. In Proceedings of the 19th International Conference on Cryptographic Hardware and Embedded Systems. 321–345. 





Joan Daemen, Seth Hoffert, Michaël Peeters, Gilles Van Assche, and Ronny Van Keer. 2020. Xoodyak, a lightweight cryptographic scheme. IACR Trans. Symmetric Cryptol. (2020), 60–87. 





Mingyang Liu, Fu Song, and Taolue Chen. 2023. Automated Verification of Correctness for Masked Arithmetic Programs. In Proceedings of the 35th International Conference on Computer Aided Verification (CAV), Part III (Lecture Notes in Computer Science, Vol. 13966), Constantin Enea and Akash Lal (Eds.). Springer, 255–280. https://doi.org/10.1007/978-3-031-37709- 9_13 





Xuzi Wang, Baofeng Wu, Lin Hou, and Dongdai Lin. 2018. Automatic Search for Related-Key Differential Trails in SIMONlike Block Ciphers Based on MILP. In Proceedings of the 21st International Conference on Information Security, Liqun Chen, Mark Manulis, and Steve A. Schneider (Eds.). 116–131. 





Zhenzhen Bao, Avik Chakraborti, Nilanjan Datta, Jian Guo, Mridul Nandi, Thomas Peyrin, and Kan Yasuda. 2019. PHOTONbeetle authenticated encryption and hash family. NIST Lightweight Compet. Round (2019), 115. 





Joan Daemen and Vincent Rijmen. 1999. AES proposal: Rijndael. (1999). 





Yu Liu, Huicong Liang, Muzhou Li, Luning Huang, Kai Hu, Chenhe Yang, and Meiqin Wang. 2021. STP models of optimal differential and linear trail for S-box based ciphers. Science China Information Sciences 64, 5 (2021). 





Hongjun Wu and Tao Huang. 2019. TinyJAMBU: A family of lightweight authenticated encryption algorithms. Submission to the NIST Lightweight Cryptography Standardization Process (2019). 





Zhenzhen Bao, Wentao Zhang, and Dongdai Lin. 2014. Speeding up the search algorithm for the best differential and best linear trails. In Proceedings of the International Conference on Information Security and Cryptology. 259–285. 





Albert Danial. 2021. cloc: v1.92. https://doi.org/10.5281/zenodo.5760077 





Zhengbin Liu, Yongqiang Li, and Mingsheng Wang. 2017. Optimal Differential Trails in SIMON-like Ciphers. IACR Trans. Symmetric Cryptol. 2017 (2017), 358–379. 





Shengbao Wu and Mingsheng Wang. 2012. Automatic search of truncated impossible differentials for word-oriented block ciphers. In Proceedings of the International Conference on Cryptology in India. 283–302. 





Ray Beaulieu, Douglas Shors, Jason Smith, Stefan Treatman-Clark, Bryan Weeks, and Louis Wingers. 2015. The SIMON and SPECK lightweight block ciphers. In Proceedings of the 52nd Annual Design Automation Conference. 175:1–175:6. 





Christoph Dobraunig, Maria Eichlseder, Stefan Mangard, Florian Mendel, Bart Mennink, Robert Primas, and Thomas Unterluggauer. 2020. ISAP v2.0. IACR Trans. Symmetric Cryptol. (2020), 390–416. 





Mohammad Mahzoun, Liliya Kraleva, Raluca Posteuca, and Tomer Ashur. 2022. Differential Cryptanalysis of K-Cipher. In IEEE Symposium on Computers and Communications. 1–7. 





Wenling Wu and Lei Zhang. 2011. LBlock: a lightweight block cipher. In Proceedings of the International conference on applied cryptography and network security. 327–344. 





Christof Beierle, Alex Biryukov, Luan Cardoso dos Santos, Johann Großschädl, Léo Perrin, Aleksei Udovenko, Vesselin Velichkov, and Qingju Wang. 2020. Alzette: A 64-Bit ARX-box - (Feat. CRAX and TRAX). In Proceedings of 40th Annual InternationalCryptology Conference. 419–448. 





Christoph Dobraunig, Maria Eichlseder, Florian Mendel, and Martin Schläffer. 2016. ASCON v1. 2. Submission to the CAESAR Competition (2016). 





Rusydi H Makarim and Raghvendra Rohit. 2022. Towards Tight Differential Bounds of ASCON: A Hybrid Usage of SMT and MILP. IACR Transactions on Symmetric Cryptology (2022), 303–340. 





Jun Yin, Chuyan Ma, Lijun Lyu, Jian Song, Guang Zeng, Chuangui Ma, and Fushan Wei. 2017. Improved cryptanalysis of an ISO standard lightweight block cipher with refined MILP modelling. In Proceedings of the International Conference on Information Security and Cryptology. 404–426. 





Christof Beierle, Alex Biryukov, Luan Cardoso dos Santos, Johann Großschädl, Léo Perrin, Aleksei Udovenko, Vesselin Velichkov, Qingju Wang, and Alex Biryukov. 2019. Schwaemm and Esch: lightweight authenticated encryption and hashing using the sparkle permutation family. NIST round 2 (2019). 





Dirk Fox. 2000. Data Encryption Standard (DES). Datenschutz und Datensicherheit (2000). 





Mitsuru Matsui. 1994. On correlation between the order of S-boxes and the strength of DES. In Proceedings of the Workshop on the Theory and Application of of Cryptographic Techniques. 366–375. 





Pei Zhang and Wenying Zhang. 2018. Differential cryptanalysis on block cipher skinny with MILP program. Security and Communication Networks 2018 (2018). 





Christof Beierle, Jérémy Jean, Stefan Kölbl, Gregor Leander, Amir Moradi, Thomas Peyrin, Yu Sasaki, Pascal Sasdrich, and Siang Meng Sim. 2016. The SKINNY family of block ciphers and its low-latency variant MANTIS. In Proceedings of the Annual International Cryptology Conference. 123–153. 





Kai Fu, Meiqin Wang, Yinghua Guo, Siwei Sun, and Lei Hu. 2016. MILP-based automatic search algorithms for differential and linear trails for speck. In Proceedings of the International Conference on Fast Software Encryption. 268–288. 





Darius Mercadier and Pierre-Évariste Dagand. 2019. Usuba: high-throughput and constant-time ciphers, by construction. In Proceedings of the 40th ACM SIGPLAN Conference on Programming Language Design and Implementation, Kathryn S. McKinley and Kathleen Fisher (Eds.). ACM, 157–173. https://doi.org/10.1145/3314221.3314636 





Wentao Zhang, Zhenzhen Bao, Dongdai Lin, Vincent Rijmen, Bohan Yang, and Ingrid Verbauwhede. 2015. RECTANGLE: a bit-slice lightweight block cipher suitable for multiple platforms. Science China Information Sciences 58, 12 (2015), 1–15. 





Tim Beyne, Yu Long Chen, Christoph Dobraunig, and Bart Mennink. 2020. Dumbo, Jumbo, and Delirium: Parallel Authenticated Encryption for the Lightweight Circus. IACR Trans. Symmetric Cryptol. (2020), 5–30. 





Pengfei Gao, Hongyi Xie, Fu Song, and Taolue Chen. 2021. A Hybrid Approach to Formal Verification of Higher-Order Masked Arithmetic Programs. ACM Trans. Softw. Eng. Methodol. 30, 3 (2021), 26:1–26:42. https://doi.org/10.1145/3428015 





Nicky Mouha and Bart Preneel. 2013. Towards finding optimal differential characteristics for ARX: Application to Salsa20. Cryptology ePrint Archive (2013). 





Yingjie Zhang, Siwei Sun, Jiahao Cai, and Lei Hu. 2018. Speeding up MILP aided differential characteristic search with Matsui’s strategy. In Proceedings of the International Conference on Information Security. 101–115. 





Eli Biham and Adi Shamir. 1990. Differential Cryptanalysis of DES-like Cryptosystems. In Proceedings of the 10th Annual International Cryptology Conference. 2–21. 





Pengfei Gao, Hongyi Xie, Pu Sun, Jun Zhang, Fu Song, and Taolue Chen. 2022. Formal Verification of Masking Countermeasures for Arithmetic Programs. IEEE Trans. Software Eng. 48, 3 (2022), 973–1000. https://doi.org/10.1109/TSE.2020.3008852 





Nicky Mouha, Qingju Wang, Dawu Gu, and Bart Preneel. 2011. Differential and linear cryptanalysis using mixed-integer linear programming. In Proceedings of the International Conference on Information Security and Cryptology. 57–76. 





Chunning Zhou, Wentao Zhang, Tianyou Ding, and Zejun Xiang. 2019. Improving the MILP-based security evaluation algorithm against differential/linear cryptanalysis using a divide-and-conquer approach. IACR Transactions on Symmetric Cryptology (2019), 438–469. 





Alex Biryukov and Christophe De Cannière. 2011. Linear cryptanalysis for block ciphers. Encyclopedia of cryptography and security (2011), 722–725. 





Zheng Gong, Svetla Nikova, and Yee Wei Law. 2011. KLEIN: a new family of lightweight block ciphers. In Proceedings of the International Workshop on Radio Frequency Identification: Security and Privacy Issues. 1–18. 





Vu Nguyen, Sophia Deeds-Rubin, Thomas Tan, and Barry Boehm. 2007. A SLOC counting standard. In Cocomo ii forum, Vol. 2007. 1–16. 





LLC Gurobi Optimization. 2018. Gurobi optimizer reference manual. 





NIST. 2023. Finalists of NIST lightweight cryptography standardization process. https://csrc.nist.gov/Projects/lightweightcryptography/finalists. 





Martin Hell, Thomas Johansson, Alexander Maximov, Willi Meier, and Hirotaka Yoshida. 2021. Grain-128AEADv2: Strengthening the Initialization Against Key Reconstruction. In Proceedings of the 20th International Conference on Cryptology and Network Security. 24–41. 





Kaisa Nyberg. 1996. Generalized feistel networks. In Proceedings of the International conference on the theory and application of cryptology and information security. 91–104. 





Howard M Heys. 2002a. A tutorial on linear and differential cryptanalysis. Cryptologia (2002), 189–221. 





Yu Sasaki and Yosuke Todo. 2017. New algorithm for modeling S-box in MILP based differential and division trail search. In Proceedings of the International Conference for Information Technology and Communications. 150–165. 





Howard M. Heys. 2002b. A Tutorial on Linear and Differential Cryptanalysis. Cryptologia 26, 3 (2002), 189–221. 





Claude E. Shannon. 1949. Communication theory of secrecy systems. Bell System Technical Journal 28, 4 (1949), 656–715. 





Howard M. Heys and Stafford E. Tavares. 1996. Substitution-Permutation Networks Resistant to Differential and Linear Cryptanalysis. J. Cryptol. (1996), 1–19. 





Kyoji Shibutani, Takanori Isobe, Harunaga Hiwatari, Atsushi Mitsuda, Toru Akishita, and Taizo Shirai. 2011. Piccolo: an ultra-lightweight blockcipher. In Proceedings of the International workshop on cryptographic hardware and embedded systems. 342–357. 





Murat Burhan Ilter and Ali Aydin Selçuk. 2021. A New MILP Model for Matrix Multiplications with Applications to KLEIN and PRINCE. In Proceedings of the 18th International Conference on Security and Cryptography. 420–427. 





Taizo Shirai, Kyoji Shibutani, Toru Akishita, Shiho Moriai, and Tetsu Iwata. 2007. The 128-Bit Blockcipher CLEFIA (Extended Abstract). In Proceedings of the 14th International Workshop on Fast Software Encryption (FSE), Revised Selected Papers. 181–195. 





Tetsu Iwata, Mustafa Khairallah, Kazuhiko Minematsu, and Thomas Peyrin. 2020. Duel of the Titans: The Romulus and Remus Families of Lightweight AEAD Algorithms. IACR Trans. Symmetric Cryptol. (2020), 43–120. 



## A A BRIEF INTRODUCTION OF BFN AND SPN

SPN cipher. Assume $\mathcal { k } = m \ell$ for some positive integers ?? and ℓ. An SPN cipher is built from permutations $\pi _ { s } : \mathbb { B } ^ { \ell } \to \mathbb { B } ^ { \ell }$ and $\pi _ { \mathfrak { p } } : \mathbb { B } ^ { \ell } \to \mathbb { B } ^ { \ell }$ where $\pi _ { s } ( { \mathrm { a . k . a . } } S { \mathrm { - b o x } } )$ is a non-linear transformation to perform a substitution for ℓ-bitstreams, and $\pi _ { \mathbf { p } }$ (a.k.a. mixing permutation) is a linear transformation. More specifically, for every $1 \leq i \leq \mathcal { F }$ and every input $X = Y ^ { 1 } \| \cdot \cdot \cdot \| Y ^ { m } , \mathsf { E n c } _ { i } ( K ^ { i } , X )$ is given by $\mathsf { E n c } _ { i } ^ { \prime } ( X ) \oplus K ^ { i }$ , where $\mathsf { E n c } _ { i } ^ { \prime }$ typically is the identify function (used as the first round) or a function built from $\pi _ { \mathbf { p } }$ and $\pi _ { s }$ such as $\mathsf { E n c } _ { i } ^ { \prime } ( Y ^ { 1 } \parallel \cdot \cdot \cdot \parallel Y ^ { m } ) \ = \ \pi _ { \mathsf { p } } ( \pi _ { \mathsf { s } } ( Y ^ { 1 } ) \parallel \cdot \cdot \cdot \parallel \pi _ { \mathsf { s } } ( Y ^ { m } ) )$ and E $\mathsf { r c } _ { i } ^ { \prime } ( Y ^ { 1 } \| \cdot \cdot \cdot \| Y ^ { m } ) = \pi _ { s } ( Y ^ { 1 } ) \mathsf { \bar { \| } } \cdot \cdot \cdot \| \pi _ { s } ( Y ^ { m } )$ (used as the final round). For SPN ciphers, S-boxes must be invertible $( \mathrm { i } . \mathrm { e } . , \pi _ { s }$ should be bijective) and linear transformations $\pi _ { \mathbf { p } }$ are given by invertible matrices or reordering of ??-bitstreams), thus En $\boldsymbol { \mathsf { z } } _ { i } ^ { - 1 } ( K ^ { i } , \cdot )$ (hence Dec) can be built trivially. 


Table 9. The DDT $\mathcal { D } _ { S }$ of the 4-bit S-box in Table 8.


<table><tr><td><eq>\Delta X</eq>\<eq>\Delta Y</eq></td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td>0</td><td>16</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>1</td><td>0</td><td>0</td><td>0</td><td>4</td><td>0</td><td>0</td><td>0</td><td>4</td><td>0</td><td>4</td><td>0</td><td>0</td><td>0</td><td>4</td><td>0</td><td>0</td></tr><tr><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>4</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>2</td><td>2</td><td>2</td><td>0</td></tr><tr><td>3</td><td>0</td><td>2</td><td>0</td><td>2</td><td>2</td><td>0</td><td>4</td><td>2</td><td>0</td><td>0</td><td>2</td><td>2</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>4</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>4</td><td>2</td><td>2</td><td>0</td><td>2</td><td>2</td><td>0</td><td>2</td><td>0</td><td>2</td><td>0</td></tr><tr><td>5</td><td>0</td><td>2</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>0</td><td>0</td><td>2</td><td>2</td><td>2</td><td>4</td><td>2</td><td>0</td><td>0</td></tr><tr><td>6</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>2</td><td>0</td><td>0</td><td>4</td><td>2</td><td>0</td><td>0</td><td>4</td></tr><tr><td>7</td><td>0</td><td>4</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>4</td></tr><tr><td>8</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>2</td><td>0</td><td>4</td><td>0</td><td>2</td><td>0</td><td>4</td></tr><tr><td>9</td><td>0</td><td>0</td><td>2</td><td>0</td><td>4</td><td>0</td><td>2</td><td>0</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td><td>4</td><td>0</td></tr><tr><td>10</td><td>0</td><td>0</td><td>2</td><td>2</td><td>0</td><td>4</td><td>0</td><td>0</td><td>2</td><td>0</td><td>2</td><td>0</td><td>0</td><td>2</td><td>2</td><td>0</td></tr><tr><td>11</td><td>0</td><td>2</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>0</td><td>4</td><td>2</td><td>2</td><td>2</td><td>0</td><td>2</td><td>0</td><td>0</td></tr><tr><td>12</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>4</td><td>0</td><td>2</td><td>2</td><td>2</td><td>2</td><td>0</td><td>0</td><td>0</td><td>2</td><td>0</td></tr><tr><td>13</td><td>0</td><td>2</td><td>4</td><td>2</td><td>2</td><td>0</td><td>0</td><td>2</td><td>0</td><td>0</td><td>2</td><td>2</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>14</td><td>0</td><td>0</td><td>2</td><td>2</td><td>0</td><td>0</td><td>2</td><td>2</td><td>2</td><td>2</td><td>0</td><td>0</td><td>2</td><td>2</td><td>0</td><td>0</td></tr><tr><td>15</td><td>0</td><td>4</td><td>0</td><td>0</td><td>4</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>4</td><td>4</td></tr></table>

Figure 10(1) shows one internal round of an SPN cipher. Table 8 shows a 4-bit S-box of PRESENT [Bogdanov et al. 2007] whose DDT is shown in Table 9. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/20671ceeaf6169a38781d2d5016c3ff6622eff6e8dfb07f7fe50c742fa96227e.jpg)


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/ba41a01d486f29025d95e0d2ed124cf20a545988941e0f5a68733c50da3c9987.jpg)



Fig. 10. One internal round of SPN and BFN.



Table 8. A 4-bit S-box of PRESENT [Bogdanov et al. 2007].


<table><tr><td>input</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td>output</td><td>12</td><td>5</td><td>6</td><td>11</td><td>9</td><td>0</td><td>10</td><td>13</td><td>3</td><td>14</td><td>15</td><td>8</td><td>4</td><td>7</td><td>1</td><td>2</td></tr></table>

BFN cipher. BFN ciphers have the same low-level building blocks (S-boxes, permutations and key schedule) as SPN ciphers, but differ in the high-level design. Furthermore, although BFN ciphers should be invertible, the involved S-boxes are not necessarily so. More specifically, an input $X \in \mathbb { B } ^ { \mathcal { Q } }$ to a BFN cipher is separated into two halves, $X _ { L } \ \in \ \mathbb { B } ^ { \frac { \mathcal { a } } { 2 } }$ and $X _ { R } \in \mathbb { B } ^ { \frac { \theta } { 2 } }$ , and $\mathsf { E n c } _ { i } ( K ^ { i } , X _ { L } \| X _ { R } )$ is given by $X _ { R } \| ( \mathsf { E n c } _ { i } ^ { \prime } ( X _ { R } ) \oplus X _ { L } \oplus K ^ { i } )$ , where Enc′?? is typically implemented using S-boxes and mixing permutations, similar to SPN ciphers. Figure 10(2) shows one internal round of a BFN cipher. While Enc′?? (?? ) may not be invertible, $\mathsf { E n c } _ { i } ^ { - 1 } ( K ^ { i } , X _ { L } \| X _ { R } )$ can be computed via $X _ { R }$ ⊕ Enc′?? (????) ⊕ $K ^ { i } \| X _ { L }$ . 

Note that there are other design approaches of iterative block ciphers, e.g., the Lai-Massey scheme [Lai and Massey 1990] and generalized Feistel schemes [Shirai et al. 2007]. Though they are less popular than the SPN and BFN schemes [Bogdanov 2010], our approach is of generic nature and could be applied to block ciphers in the other schemes, thanks to the expressivity of EasyBC. 

## B KEY RECOVERING FROM OPTIMAL DIFFERENTIAL CHARACTERISTICS

Consider the optimal (?? − 1)-round differential characteristic $( \Delta X ^ { 0 } , \cdots , \Delta X ^ { \star - 1 } )$ . The attacker first can randomly select a plaintext $X ^ { 0 }$ , computes $X ^ { 0 } \oplus \Delta X ^ { 0 }$ and two ciphertexts $\mathsf { E n c } ( K , X ^ { 0 } )$ and Enc $( K , X ^ { 0 } \oplus \Delta X ^ { 0 } )$ . Then, the attacker will try to solve the following equation for the subkey $K ^ { \mathcal { F } }$ : 

$$
\mathsf {E n c} _ {\nu} ^ {- 1} (K ^ {\nu}, \mathsf {E n c} (K, X ^ {0})) \oplus \mathsf {E n c} _ {\nu} ^ {- 1} (K ^ {\nu}, \mathsf {E n c} (K, X ^ {0} \oplus \Delta X ^ {0})) = \Delta X ^ {\nu - 1}.
$$

The solutions of the equation are candidates of the subkey $K ^ { \mathcal { \mathcal { F } } }$ . The attacker can uniformly sample a number of plaintexts and increases the counter of each candidate subkey fulfilling the above equation. The candidate being counted significantly more often than others is regarded as the actual subkey $K ^ { \mathcal { F } }$ , as the differential characteristic $( \Delta X ^ { 0 } , \cdots , \Delta X ^ { \star - 1 } )$ is optimal. 

After recovering $K ^ { \mathcal { \mathcal { F } } }$ , the attacker can iteratively recover the subkey $K ^ { i + 1 }$ according to the optimal ??-th round differential characteristic $( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { i } )$ for $i < \varkappa - 1$ by solving the following equation, 

$$
\mathsf {E n c} _ {i + 1} ^ {- 1} (K ^ {i + 1}, X ^ {i + 1})) \oplus \mathsf {E n c} _ {i + 1} ^ {- 1} (K ^ {i + 1}, X ^ {\prime i + 1}) = \Delta X ^ {i},
$$

where the outputs $X ^ { i + 1 }$ and $X ^ { \prime 1 + 1 }$ of the $( i + 1 )$ -round, i.e., the inputs of the $( i + 2 )$ -round, can be obtained from the ciphertexts $\mathsf { E n c } ( K , X ^ { 0 } )$ and $\mathsf { E n c } ( K , X ^ { 0 } \oplus \Delta X ^ { 0 } )$ using the recovered subkeys $K ^ { i + 2 } , \cdot \cdot \cdot , K ^ { \mathcal { r } }$ . The number of plaintexts required to determine the (??+1)-round subkey $K ^ { i + 1 }$ is proportional to $\frac { 1 } { \mathsf { P r } ( \widetilde { \Delta X } ^ { 0 } , \cdots , \widetilde { \Delta X } ^ { i } ) }$ for the optimal ??-th round differential characteristic $( \widetilde { \Delta X } ^ { 0 } , \cdots , \widetilde { \Delta X } ^ { i } )$ [Heys 2002b]. 

## C MISSING EXAMPLES

## C.1 Illustrating Example for Section 4.1

Consider the function $f _ { \oplus } : \mathbb { B } ^ { n } \times \mathbb { B } ^ { n } \to \mathbb { B } ^ { n }$ such that $f _ { \oplus } ( X ^ { 1 } , X ^ { 2 } ) = X ^ { 1 } \oplus X ^ { 2 }$ . We will construct the SMT formula $\phi _ { f _ { \oplus } } : = \phi _ { 1 } \wedge \phi _ { 2 }$ , where 

$$
\phi_ {1} := \big ((\Delta X ^ {1} \neq 0 \lor \Delta X ^ {2} \neq 0) \land Y = X ^ {1} \oplus X ^ {2} \land Y \oplus \Delta Y = (X ^ {1} \oplus \Delta X ^ {1}) \oplus (X ^ {2} \oplus \Delta X ^ {2}) \big),
$$

$$
\psi_ {2} := \left\{ \begin{array}{c} (d = c _ {1} + c _ {2} + c _ {3}) \\ \wedge \\ \big ((\Delta X ^ {1} \neq 0 \land c _ {1} = 1) \lor (\Delta X ^ {1} = 0 \land c _ {1} = 0) \big) \\ \wedge \\ \big ((\Delta X ^ {2} \neq 0 \land c _ {2} = 1) \lor (\Delta X ^ {2} = 0 \land c _ {2} = 0) \big) \\ \wedge \\ \big ((\Delta Y \neq 0 \land c _ {3} = 1) \lor (\Delta Y = 0 \land c _ {3} = 0) \big) \end{array} \right\},
$$

$X ^ { 1 } , X ^ { 2 } , Y , \Delta X ^ { 1 } , \Delta X ^ { 2 } , \Delta Y$ are n-bit unsigned integer variables, and $d , c _ { 1 } , c _ { 2 } , c _ { 2 }$ are 2-bit unsigned integer variables. 

The minimized (resp. maximized) value of ?? subject to the SMT formula $\phi _ { f _ { \oplus } }$ is $\mathcal { B } _ { \mathsf { w w } } ^ { \operatorname* { m i n } } ( f _ { \oplus } )$ (resp. $\mathcal { B } _ { \mathsf { w w } } ^ { \mathrm { m a x } } ( f _ { \oplus } ) )$ , that is 2 (resp. 3). 

## C.2 Illustrating Example for Section 4.3

Consider the S-box S shown in Table 8 whose DDT is shown in Table 9. There are three nonzero probabilities $\mathsf { P r } _ { S } ( \Delta X , \Delta Y )$ for $( \Delta X , \Delta Y ) \in \mathbb { B } ^ { 4 } \times \mathbb { B } ^ { 4 }$ , namely, $2 ^ { - 0 } ( \mathrm { i . e . , } 1 6 / 1 6 ) , 2 ^ { - 3 } ( \mathrm { i . e . , } 2 / 1 6 )$ and $2 ^ { - 2 }$ (i.e., 4/16). Thus, $V = \{ 2 ^ { - 0 } , 2 ^ { - 3 } , 2 ^ { - 2 } \}$ . 


Table 10. Part of the extended DDT $\mathcal { D } _ { S } ^ { \dagger }$ of the 4-bit S-box in Table 8.


<table><tr><td><eq>\Delta X\ b_1\ b_2</eq></td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td>0 0 0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>0 0 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>0 1 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>0 1 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>1 0 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>1 0 1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td></tr><tr><td>1 1 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>1 1 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>2 0 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>2 0 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>2 1 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>2 1 1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td></tr><tr><td>3 0 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>3 0 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>3 1 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>3 1 1</td><td>0</td><td>1</td><td>0</td><td>1</td><td>1</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>4 0 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>4 0 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>4 1 0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td></tr><tr><td>4 1 1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>1</td><td>1</td><td>0</td><td>1</td><td>0</td><td>1</td><td>0</td></tr></table>

The MaxSMT problem for the S-box S is defined as $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ , where 

$$
\Phi_ {1} ^ {\mathcal {S}} := \left\{ \begin{array}{c} c _ {1} \cdot p _ {1, 1} + c _ {2} \cdot p _ {2, 1} + c _ {3} \cdot p _ {3, 1} = 0 \\ \wedge \\ c _ {1} \cdot p _ {1, 2} + c _ {2} \cdot p _ {2, 2} + c _ {3} \cdot p _ {3, 2} = 3 \\ \wedge \\ c _ {1} \cdot p _ {1, 3} + c _ {2} \cdot p _ {2, 3} + c _ {3} \cdot p _ {3, 3} = 2 \end{array} \right\},
$$

$$
\Phi_ {2} ^ {\mathcal {S}} := \left\{c _ {1} = 0, c _ {2} = 0, c _ {3} = 0 \right\}.
$$

We can see that the following assginment is a solution of the MaxSMT problem $( \Phi _ { 1 } ^ { S } , \Phi _ { 2 } ^ { S } )$ : 

$$
\left\{ \begin{array}{c} p _ {1, 1} = 0, p _ {2, 1} = 0, p _ {3, 1} = 0, \\ p _ {1, 2} = 1, p _ {2, 2} = 1, p _ {3, 2} = 0, \\ p _ {1, 3} = 0, p _ {2, 3} = 1, p _ {3, 3} = 0, \\ c _ {1} = 1, c _ {2} = 2, c _ {3} = 0 \end{array} \right\}.
$$

By introducing two addition Boolean variables, $b _ { 1 }$ and $b _ { 2 }$ , we can construct the extended DDT $\mathcal { D } _ { s } ^ { \dagger }$ of the S-box S, part of which is shown in Table 10. 

## D MISSING PROOFS

## D.1 Proof of Lemma 5.1

Lemma 5.1. Suppose $\mathbb { [ } e \mathbb { J } _ { \gamma } ^ { \mathsf { w } } = \left( \Psi , \vec { b } \right)$ with $\Psi \ne \emptyset$ . The assignment $\{ b _ { 1 } , \cdots , b _ { m } \}$ is a solution $o f \Psi$ $i f f \left\{ b _ { 1 } , \cdots , b _ { i } \right\}$ is feasible differences of the operands and result of ??, where $\{ b _ { i + 1 } , \cdots , b _ { m } \}$ is the assignment of the auxiliary Boolean variables if exist.. 

Proof. Suppose $\mathbb { I } e \mathbb { I } _ { \gamma } ^ { \mathsf { w } } = ( \Psi , \vec { b } )$ . If ?? is of the form View $( x , i , j ) , \sim x \mathrm { o r } x \langle \cdot y \cdot \rangle , \Psi = 0$ . Below, we consider the other cases. 

$\bullet \ e$ is $x _ { 1 } \odot x _ { 2 }$ for $\odot \in \{ + , - , \oplus \}$ . Suppose $\gamma ( x _ { 1 } ) = b _ { 1 }$ and $\gamma ( x _ { 2 } ) = b _ { 2 }$ , namely, $b _ { 1 }$ and $b _ { 2 }$ model the differences of the operands $x _ { 1 }$ and $x _ { 2 }$ respectively. Let $b _ { 0 }$ be the Boolean variable that models the difference of the result of $x _ { 1 } \odot x _ { 2 }$ . Recall that Ψ can be two equivalent sets of IL constraints $\Psi _ { 2 , 3 } ^ { 1 } ( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ and $\Psi _ { 2 , 3 } ^ { 2 } \left( b _ { 0 } , b _ { 1 } , b _ { 2 } \right)$ . 

Since $\mathcal { B } _ { \mathsf { w w } , \odot } ^ { \operatorname* { m a x } } = 3$ and $\mathcal { B } _ { \mathsf { w w } , \odot } ^ { \operatorname* { m i n } } = 2$ (cf. Table 2), we get that 

– either at least two of $b _ { 0 } , b _ { 1 } , b _ { 2 }$ are 1, 

$- \mathrm { o r } b _ { 0 } = b _ { 1 } = b _ { 2 } = 0 .$ . 

It is easy to see that $\left( { b _ { 0 } , b _ { 1 } , b _ { 2 } } \right)$ is feasible differences of operands $x _ { 1 } , x _ { 2 }$ and result of $x _ { 1 } \odot x _ { 2 }$ iff $( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ satisfies $\Psi _ { 2 , 3 } ^ { 1 } ( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ . For $\Psi _ { 2 , 3 } ^ { 2 } ( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ , the auxiliary Boolean variable $b ^ { \prime }$ is 0 iff $b _ { 0 } = b _ { 1 } = b _ { 2 } = 0$ . The constraint $\begin{array} { r } { \sum _ { i = 0 } ^ { 2 } b _ { i } \geq 2 b ^ { \prime } } \end{array}$ exactly characterizes that at least two of $b _ { 0 } , b _ { 1 } , b _ { 2 }$ are 1 if $b ^ { \prime } = 1$ . Thus, $( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ is feasible differences of the operands $x _ { 1 } , x _ { 2 }$ and result of $x _ { 1 } \odot x _ { 2 }$ iff $( b _ { 0 } , b _ { 1 } , b _ { 2 } , b ^ { \prime } )$ satisfies $\Psi _ { 2 . 3 } ^ { 2 } \big ( b _ { 0 } , b _ { 1 } , b _ { 2 } \big )$ , where $b ^ { \prime } = 0$ iff $b _ { 0 } = b _ { 1 } = b _ { 2 } = 0$ . 

• ?? is $x _ { 1 } \odot x _ { 2 }$ for $\odot \in \{ \land , \lor \}$ . Suppose $\gamma ( x _ { 1 } ) = b _ { 1 }$ and $\gamma ( x _ { 2 } ) = b _ { 2 }$ , namely, $b _ { 1 }$ and $b _ { 2 }$ model the differences of the operands $x _ { 1 }$ and $x _ { 2 }$ respectively. Let $b _ { 0 }$ be the Boolean variable that models the difference of the result of $x _ { 1 } \odot x _ { 2 }$ . 

Since $\mathcal { B } _ { \mathsf { w w } , \odot } ^ { \operatorname* { m a x } } = 3$ and $\mathcal { B } _ { \mathsf { w w } , \odot } ^ { \operatorname* { m i n } } = 1$ (cf. Table 2), we have that 

– either at least one of $\dot { b } _ { 0 } , b _ { 1 } , b _ { 2 }$ is 1, 

$- \mathrm { o r } b _ { 0 } = b _ { 1 } = b _ { 2 } = 0 .$ . 

Moreover, $b _ { 0 } = 0$ if $b _ { 1 } = b _ { 2 } = 0$ . The constraint $b _ { 1 } + b _ { 2 } \geq b _ { 0 }$ exactly characterizes that $b _ { 0 } = 0$ if both $b _ { 1 }$ and $b _ { 2 }$ are 0, otherwise $b _ { 0 }$ can be 1 or 0. Thus, $( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ is feasible differences of the operands $x _ { 1 } , x _ { 2 }$ and result of $x _ { 1 } \odot x _ { 2 } { \mathrm { i f f ~ } } ( b _ { 0 } , b _ { 1 } , b _ { 2 } )$ satisfies $b _ { 1 } + b _ { 2 } \geq b _ { 0 }$ . 

• ?? is $M * x .$ Suppose $\boldsymbol { \gamma } ( \boldsymbol { x } ) = \vec { b } , \mathrm { i . e . , } \vec { b }$ is a vector of Boolean variables each of which models the difference of an entry in the array ??. Let $\vec { b ^ { \prime } }$ be the vector of Boolean variables each of which models the difference of an entry in the resulting array $M \odot x$ . Recall that Ψ can be two equivalent $\Psi _ { M } ^ { 1 } ( \vec { b } , \vec { b ^ { \prime } } )$ $\Psi _ { M } ^ { 2 } ( \vec { b } , \vec { b ^ { \prime } } )$ $\mathcal { B } _ { \mathsf { w w } , M } ^ { \operatorname* { m i n } } \geq 1$ 

We observe that 

– either the sum of their differences $( \vec { b } , \vec { b } ^ { \prime } )$ ranges from $\mathcal { B } _ { \mathsf { w w } , M } ^ { \operatorname* { m i n } }$ to Bmax , $\mathcal { B } _ { \mathsf { w w } , M } ^ { \mathrm { m a x } } .$ ww,?? namely, 

$$
\mathcal {B} _ {\mathrm{ww}, M} ^ {\min} \leq \sum_ {i = 0} ^ {| x | - 1} (\vec {b} _ {i} + \vec {b} _ {i} ^ {\prime}) \leq \mathcal {B} _ {\mathrm{ww}, M} ^ {\max},
$$

$M \ast x$ $\begin{array} { r } { ( \mathrm { i . e . , ~ } \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) = 0 ) } \end{array}$ 

These two conditions are equivalent to $\Psi _ { M } ^ { i } ( \vec { b } , \vec { b ^ { \prime } } )$ for $i \in \{ 1 , 2 \}$ . Indeed, in $\Psi _ { M } ^ { 1 } ( \vec { b } , \vec { b ^ { \prime } } ) , b ^ { \prime \prime }$ is an auxiliary Boolean variable, 

$\begin{array} { r } { - \ 2 | x | \cdot b ^ { \prime \prime } \geq \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) } \end{array}$ exactly characterizes that $\begin{array} { r } { b ^ { \prime \prime } = 1 \ \mathrm { i f } \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) \neq 0 ; } \end{array}$ 

$\begin{array} { r } { - \ B _ { \mathrm { w w } , M } ^ { \mathrm { m i n } } \cdot b ^ { \prime \prime } \leq \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) \leq \mathcal { B } _ { \mathrm { w w } , M } ^ { \mathrm { m a x } } } \end{array}$ exactly characterizes $\begin{array} { r } { \mathcal { B } _ { \mathrm { w w } , M } ^ { \operatorname* { m i n } } \leq \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) \leq \mathcal { B } _ { \mathrm { w w } , M } ^ { \operatorname* { m a x } } } \end{array}$ $b ^ { \prime \prime } = 1$ $\begin{array} { r } { b ^ { \prime \prime } = 0 \operatorname { i f } \sum _ { i = 0 } ^ { | x | - 1 } ( { \vec { b } } _ { i } + { \vec { b } } _ { i } ^ { \prime } ) = 0 } \end{array}$ . 

The set $\Psi _ { M } ^ { 2 } ( \vec { b } , \vec { b ^ { \prime } } )$ is similar to $\Psi _ { M } ^ { 1 } ( \vec { b } , \vec { b ^ { \prime } } )$ except that $\begin{array} { r } { 2 | { \boldsymbol x } | \cdot b ^ { \prime \prime } \geq \sum _ { i = 0 } ^ { | { \boldsymbol x } | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) } \end{array}$ is equivalently expressed by the constraints $b ^ { \prime \prime } \geq \vec { b } _ { 0 } , b ^ { \prime \prime } \geq \vec { b } _ { 0 } ^ { \prime } , \cdot \cdot \cdot , b ^ { \prime \prime } \geq \vec { b } _ { | x | - 1 } , b ^ { \prime \prime } \geq \vec { b } _ { | x | - 1 } ^ { \prime }$ . Thus, $( \vec { b } , \vec { b } ^ { \prime } )$ is feasible differences of the operand ?? and result of $M \ast x \operatorname { i f f } { ( \vec { b } , \vec { b } ^ { \prime } , b ^ { \prime \prime } ) }$ satisfies $\Psi _ { M } ^ { i } ( \vec { b } , \vec { b ^ { \prime } } )$ , where $\begin{array} { r } { b ^ { \prime \prime } = 0 \operatorname* { i f f } \sum _ { i = 0 } ^ { | x | - 1 } ( \vec { b } _ { i } + \vec { b } _ { i } ^ { \prime } ) = 0 } \end{array}$ . 

• ?? is $x \langle y \rangle$ . Let $b = \gamma ( y )$ and $b ^ { \prime }$ be the Boolean variable that models the difference of the result of $x \langle y \rangle$ . We observe that 

– if the S-box ?? is injective, the result of $\dot { } x \langle y \rangle$ has some differences $( \mathrm { i } . \mathrm { e } . , b ^ { \prime } = 1 )$ iff ?? has some differences $( \mathrm { i . e . , } b = 1 )$ , which is equivalent to $b = b ^ { \prime }$ ; 

– if the S-box ?? is non-injective, $x \langle y \rangle$ may differ in two executions only if ?? differs in the two executions, which is equivalent to $b \geq b ^ { \prime }$ . 

Thus, $( b , b ^ { \prime } )$ is feasible differences of the operand ?? and the result of $x \langle y \rangle$ iff $b = b ^ { \prime }$ if the S-box ?? is injective, otherwise $b \geq b ^ { \prime }$ . 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/6b297120c56a9ee108234b1d8f22dda32d22f131099cddba747963e4449c59f4.jpg)


## D.2 Proof of Theorem 5.2

Theorem 5.2. Let $\ [ \boldsymbol { P } \ ] ^ { \mathsf { w } } = ( \Phi , \gamma , \Theta )$ and ?? be the minimum value of the objective function $\sum _ { b \in \Theta } b$ subject to Φ ∪ $\begin{array} { r } { \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \ge 1 \} } \end{array}$ . We have that $N \leq N _ { \mathsf { d i f f } }$ . 

Proof. Suppose $[ [ P ] ] ^ { \mathsf { w } } = ( \Phi , \gamma , \Theta )$ for the given program ?? and ?? is the minimum value of the objective function $\sum _ { b \in \Theta } b$ subject to $\begin{array} { r } { \Phi \cup \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq 1 \} } \end{array}$ . Given two pairs of inputs $( K , X )$ and $( K , X ^ { \prime } )$ such that $X \neq X ^ { \prime } ;$ , ?? executes the same sequence of statements under the inputs $( K , X )$ and $( K , X ^ { \prime } )$ . (Note that ?? is branching-free.) Let $\sigma _ { 0 } S _ { 1 } \sigma _ { 1 } S _ { 2 } \sigma _ { 2 } \cdot \cdot \cdot S _ { m } \sigma _ { m }$ and $\sigma _ { 0 } ^ { \prime } S _ { 1 } \sigma _ { 1 } ^ { \prime } S _ { 2 } \sigma _ { 2 } ^ { \prime } \cdot \cdot \cdot S _ { m } \sigma _ { m } ^ { \prime }$ be the sequences of states and statements of the executions under the inputs $( K , X )$ and $( K , X ^ { \prime } )$ , respectively. Let $b _ { x }$ be the corresponding Boolean variable of each scalar variable ?? in a state $\sigma _ { i }$ and let $b _ { x , j }$ be the corresponding Boolean variable of the $( j + 1 )$ -th entry $( \mathrm { i } . \mathrm { e } . , x _ { j } )$ in the array ?? in a state $\sigma _ { i } .$ . 

By Lemma 5.1 and induction on ?? and syntactic structure of statements and expressions, we can get that if $b _ { x } = 1 ( \mathrm { r e s p . } b _ { x , j } = 1 )$ in any solution of $\begin{array} { r } { \Phi \cup \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq 1 \} } \end{array}$ that corresponds to the two executions $\sigma _ { 0 } S _ { 1 } \sigma _ { 1 } S _ { 2 } \sigma _ { 2 } \cdot \cdot \cdot S _ { m } \sigma _ { m }$ and $\sigma _ { 0 } ^ { \prime } S _ { 1 } \sigma _ { 1 } ^ { \prime } S _ { 2 } \sigma _ { 2 } ^ { \prime } \cdot \cdot \cdot S _ { m } \sigma _ { m } ^ { \prime }$ , then $\sigma _ { i } ( x ) \oplus \sigma _ { i } ^ { \prime } ( x ) \neq 0$ (resp. $\sigma _ { i } ( x _ { j } ) \oplus \sigma _ { i } ^ { \prime } ( x _ { j } ) \neq 0 )$ . Thus, for any S-box S in ?? under those two executions, if the Boolean variable $b _ { S }$ that models the input difference of S is 1 in the solution of Φ ∪ $\begin{array} { r } { \{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \ge 1 \} } \end{array}$ , then S must have different inputs, indicating that S is active. Therefore, we can deduce that $N \leq N _ { \mathsf { d i f f } }$ . 

We remark that some solutions of Φ $\cup \left\{ ( \sum _ { i = 0 } ^ { n - 1 } \gamma ( t x t , i ) ) \geq 1 \right\}$ may yield invalid differential characteristics, thus we cannot conclude that $N = N _ { \sf d i f f }$ . □ 

## D.3 Proof of Lemma 6.1

Lemma 6.1. Suppose $\left[ [ e ] \right] _ { \gamma } ^ { \mathtt { B } } = ( \Psi , \vec { b } )$ with $\Psi \ne \emptyset$ . The assignment $\{ b _ { 1 } , \cdots , b _ { m } \}$ is a solution of Ψ iff $\{ b _ { 1 } , \cdots , b _ { i } \}$ is feasible bit-level differences of the operands and result of ??, where $\{ b _ { i + 1 } , \cdots , b _ { m } \}$ is the assignment of the auxiliary Boolean variables if exist. 

Proof. Suppose $\mathbb { I } \boldsymbol { e } \mathbb { I } _ { \boldsymbol { \gamma } } ^ { \mathtt { B } } = ( \Psi , \vec { b } )$ . If ?? is of the form View $( x , i , j ) , \sim x$ , touint $\left( x _ { 1 } , \cdots , x _ { m } \right)$ or $x \langle \cdot y \cdot \rangle$ , $\Psi = \emptyset$ . If ?? is $x \langle y \rangle$ , the result follows from Proposition 4.2. Below, we consider the other cases. 

• ?? is $x _ { 1 } \odot x _ { 2 }$ for $\odot \in \{ \land , \lor \}$ . Suppose $\gamma ( x _ { 1 } ) = \vec { b } ^ { 1 }$ and $\gamma ( x _ { 2 } ) = { \vec { b } } ^ { 2 }$ , namely, $\vec { b } ^ { 1 }$ and $\vec { b } ^ { 2 }$ model the bit-level differences of of the operands $x _ { 1 }$ and ??2 respectively. Let ${ \vec { b } } ^ { 0 }$ be the vector of Boolean variables each of which models the difference of a bit in the result of $x _ { 1 } \odot x _ { 2 }$ . 

For each $0 \leq i < \| x \|$ , for any fixed $ { \vec { b } } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ , we observe that: 

$- \ \mathrm { i f } \ \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0$ , then $\vec { b } _ { i } ^ { 0 } = 0 ;$ 

– otherwise $\vec { b } _ { i } ^ { 0 }$ could be 0 and 1 (with the probability of $\textstyle { \frac { 1 } { 2 } } )$ 

The constraint $\vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq \vec { b } _ { i } ^ { 0 }$ exactly characterizes that $\vec { b } _ { i } ^ { 0 } = 0$ if $\vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0$ otherwise $\vec { b } _ { i } ^ { 0 }$ could be 0 and 1. Thus, $\{ \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } \}$ satisfies $\vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq \vec { b } _ { i } ^ { 0 }$ iff $\{ \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } \}$ } is feasible differences of $( i + 1 )$ -th bits of the operands $x _ { 1 } , x _ { 2 }$ and result of $x _ { 1 } \odot x _ { 2 }$ . The result immediately follows. 

• ?? is $x _ { 1 } \oplus x _ { 2 }$ . Suppose $\gamma ( x _ { 1 } ) = \vec { b } ^ { 1 }$ and $\gamma ( x _ { 2 } ) = \vec { b } ^ { 2 }$ , namely, $\vec { b } ^ { 1 }$ and $\vec { b } ^ { 2 }$ model the bit-level differences of the operands $x _ { 1 }$ and $x _ { 2 }$ respectively. Let $\vec { b } ^ { 0 }$ be the vector of Boolean variables each of which models the difference of a bit in the result of $x _ { 1 } \oplus x _ { 2 }$ . Recall that Ψ can be three equivalent sets of $\cup _ { i = 0 } ^ { \| x \| - 1 } \psi _ { \oplus } ^ { 1 } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } ) , \cup _ { i = 0 } ^ { \| x \| - 1 } \psi _ { \oplus } ^ { 2 } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ and $\cup _ { i = 0 } ^ { \| x \| - 1 } \psi _ { \oplus } ^ { 3 } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ . 

For each $0 \leq i < \| x \|$ , for any fixed $ { \vec { b } } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ , we observe that: 

– either $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0 ,$ 

– or exactly two of $\vec { b } _ { i } ^ { 0 } , \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 }$ are $1 ( \mathrm { i . e . , } \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } = 2 ) .$ 

The above two conditions are exactly characterized by $\psi _ { \oplus } ^ { j } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ for any $j \in \{ 1 , 2 , 3 \}$ . Indeed, we deduce that $\{ \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } \}$ is feasible differences of $( i + 1 )$ -th bits of the operands $x _ { 1 } , x _ { 2 }$ and result of $x _ { 1 } \odot x _ { 2 } \mathrm { i f f }$ 

$\mathrm { ~ - ~ } \{ \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } , \boldsymbol { b } ^ { \prime } \}$ satisfies $\psi _ { \oplus } ^ { 1 } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ , where $\begin{array} { r } { b ^ { \prime } = 0 } \end{array}$ iff $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0 ;$ 

$\mathbf { \overline { { \Sigma } } } - \{ \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } \}$ satisfies $\psi _ { \oplus } ^ { 2 } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ ; 

$\mathbf { \overline { { \Sigma } } } - \{ \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } \}$ satisfies $\psi _ { \oplus } ^ { 3 } ( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } )$ , where $\boldsymbol { b } ^ { \prime } = 0$ iff $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0 .$ 

Thus, the result follows. 

• ?? is $x _ { 1 } \odot x _ { 2 }$ for $\odot \in \{ + , - \}$ . Suppose $\gamma ( x _ { 1 } ) = \vec { b } ^ { 1 }$ and $\gamma ( x _ { 2 } ) = \vec { b } ^ { 2 }$ , namely, $\vec { b } ^ { 1 }$ and $\vec { b } ^ { 2 }$ model the bit-level differences of the operands $x _ { 1 }$ and $x _ { 2 }$ respectively. Let $\vec { b } ^ { 0 }$ be the vector of Boolean variables each of which models the difference of a bit in the result of $x _ { 1 } \odot x _ { 2 }$ . 

Consider $y = x _ { 1 } + x _ { 2 }$ . Clearly, 

$- \sin _ { i } ( y ) = \mathtt { b i n } _ { i } ( x _ { 1 } ) \oplus \mathtt { b i n } _ { i } ( x _ { 2 } ) \oplus { \vec { c } } _ { i }$ , for every $0 \leq i < \| y \| ;$ 

– the carry bit ${ \vec { c } } _ { i } = 1 { \mathrm { ~ i f f ~ b i n } } _ { i - 1 } ( x _ { 1 } ) + { \mathrm { b i n } } _ { i - 1 } ( x _ { 2 } ) + { \vec { c } } _ { i - 1 } \geq 2 ,$ for every $1 \leq i < \| y \|$ , with ${ \vec { c } } _ { 0 } = 0$ 

Suppose $\vec { b } ^ { 0 } = \mathrm { b i n } ( y )$ ⊕ b $\mathsf { n } ( y ^ { \prime } ) = \mathsf { b i n } ( x _ { 1 } + x _ { 2 } )$ ⊕ bin $( x _ { 1 } ^ { \prime } + x _ { 2 } ^ { \prime } )$ where bin $\left( x _ { 1 } \right)$ ⊕ bin $( x _ { 1 } ^ { \prime } ) = \vec { b } ^ { 1 }$ a nd bin $\left( x _ { 2 } \right)$ ⊕ bin $( x _ { 2 } ^ { \prime } ) = \vec { b } ^ { 2 }$ . Let $ { \vec { b } } _ { i } ^ { 3 }$ be the difference of the carry bit $\vec { c } _ { i } \oplus \vec { c } _ { i } ^ { \prime }$ for every $0 \leq i < \| y \|$ . 

We first consider the case $i \geq 2 .$ . Clearly, 

$$
\vec {b} _ {i} ^ {0} = \left(\operatorname{bin} _ {i} (x _ {1}) \oplus \operatorname{bin} _ {i} (x _ {2}) \oplus \vec {c} _ {i}\right) \oplus \left(\operatorname{bin} _ {i} (x _ {1} ^ {\prime}) \oplus \operatorname{bin} _ {i} (x _ {2} ^ {\prime}) \oplus \vec {c} _ {i} ^ {\prime}\right) = \vec {b} _ {i} ^ {1} \oplus \vec {b} _ {i} ^ {2} \oplus \vec {b} _ {i} ^ {3}.
$$

Observe that $\vec { b } _ { i } ^ { 3 } = 1$ iff exactly one of the follows holds: 

$- \ \mathsf { b i n } _ { i - 1 } ( x _ { 1 } ) + \mathsf { b i n } _ { i - 1 } ( x _ { 2 } ) + \vec { c } _ { i - 1 } \geq 2 ,$ or 

$\begin{array}{c} \begin{array} { r } { - \left. \mathsf { b i n } _ { i - 1 } ( x _ { 1 } ^ { \prime } ) + \mathsf { b i n } _ { i - 1 } ( x _ { 2 } ^ { \prime } ) + \vec { c } _ { i - 1 } ^ { \prime } \geq 2 . \right.} \end{array}   \end{array}$ 

Since $\vec { c } _ { i - 1 } \oplus \vec { c } _ { i - 1 } ^ { \prime } = \vec { b } _ { i - 1 } ^ { 3 } = \vec { b } _ { i - 1 } ^ { 0 } \oplus \vec { b } _ { i - 1 } ^ { 1 } \oplus \vec { b } _ { i - 1 } ^ { 2 }$ , we can deduce that 

$- \ \mathrm { i f } \ \vec { b } _ { i - 1 } ^ { 0 } = \vec { b } _ { i - 1 } ^ { 1 } = \vec { b } _ { i - 1 } ^ { 2 } = 1$ , then $\vec { c } _ { i - 1 } \oplus \vec { c } _ { i - 1 } ^ { \prime } = \vec { b } _ { i - 1 } ^ { 3 } = \vec { b } _ { i } ^ { 3 } = 1$ and $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ 

$- \ \mathrm { i f } \ \vec { b } _ { i - 1 } ^ { 0 } = \vec { b } _ { i - 1 } ^ { 1 } = \vec { b } _ { i - 1 } ^ { 2 } = 0 ,$ , then $\vec { c } _ { i - 1 } \oplus \vec { c } _ { i - 1 } ^ { \prime } = \vec { b } _ { i - 1 } ^ { 3 } = \vec { b } _ { i } ^ { 3 } = 0$ and $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 }$ 

– otherwise $1 \leq \vec { b } _ { i - 1 } ^ { 0 } + \vec { b } _ { i - 1 } ^ { 1 } + \vec { b } _ { i - 1 } ^ { 2 } \leq 2$ . Indeed, the probability of $\vec { b } _ { i } ^ { 3 } = 1 \mathrm { ~ i s ~ } \frac { 1 } { 2 }$ , implying that the probability of $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ , (resp. $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 }$ and $\vec { b } _ { i } ^ { 0 } = 1 )$ is ${ \frac { 1 } { 2 } } .$ . 

The above three conditions are exactly characterized by the set of IL constraints $\Psi _ { i }$ for $i \geq 2 ,$ where 

$\begin{array} { r } { - \ \mathrm { i f } \sum _ { j = 0 } ^ { 2 } \vec { b } _ { i - 1 } ^ { j } = 3 , \Psi _ { i } } \end{array}$ becomes $\big \{ \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq 1 , \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } \leq \vec { b } _ { i } ^ { 2 } + 1 , \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 2 } \leq \vec { b } _ { i } ^ { 1 } + 1 , \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \leq \vec { b } _ { i } ^ { 0 } + 1 \big \}$ which is equivalent to $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ ; 

$\begin{array} { r } { - \mathrm { ~ i f ~ } \sum _ { j = 0 } ^ { 2 } \vec { b } _ { i - 1 } ^ { j } = 0 , \Psi _ { i } } \end{array}$ becomes $\big \{ - \dot { \vec { b } } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq 0 , \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } - \vec { b } _ { i } ^ { 2 } \geq 0 , \vec { b } _ { i } ^ { 0 } - \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq 0 , 2 \geq \vec { b } _ { i } ^ { 0 } + \vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \big \}$ which is equivalent to $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 }$ ; 

$\begin{array} { r } { - \mathrm { ~ i f ~ } 1 \le \sum _ { j = 0 } ^ { 2 } \vec { b } _ { i - 1 } ^ { j } \le 2 , \Psi _ { i } } \end{array}$ always holds. 

Furthermore, $\Psi _ { 0 }$ and $\Psi _ { 1 }$ can be defined similarly (with simplification by $\vec { b } _ { 0 } ^ { 3 } = \vec { c } _ { 0 } = \vec { c } _ { 0 } ^ { \prime } = 0 )$ Thus, $\{ \vec { b } ^ { 1 } , \vec { b } ^ { 2 } , \vec { b } ^ { 0 } \}$ is feasible bit-level differences of the operands $x _ { 1 } , x _ { 2 }$ and result of $x _ { 1 } + x _ { 2 }$ iff $\{ \vec { b } ^ { 1 } , \vec { b } ^ { 2 } , \vec { b } ^ { 0 } , \boldsymbol { b } ^ { \prime } \}$ $\textstyle \bigcup _ { i = 0 } ^ { \| x \| - 1 } \Psi _ { i }$ $b ^ { \prime }$ $\Psi _ { 0 }$ $b ^ { \prime }$ $\{ \vec { b } ^ { 1 } , \vec { b } ^ { 2 } , \vec { b } ^ { 0 } \}$ $\textstyle \bigcup _ { i = 0 } ^ { \| x \| - 1 } \Psi _ { i }$ 

The result of $x _ { 1 } - x _ { 2 }$ follows because $y = x _ { 1 } - x _ { 2 } \mathrm { i f f } x _ { 2 } = x _ { 1 } + y$ , and the Boolean variables $\vec { b } _ { i } ^ { 0 } , \vec { b } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ in $\Psi _ { i }$ are symmetric. 

• ?? is $M * x$ . Suppose $y = M * x$ and $s = { \frac { \| x \| } { | x | } }$ (i.e., the bit width of entries in the arrays ?? and $y )$ . Let $\gamma ( x ) = { \vec { b } } ,$ i.e., for every $0 \leq i < | x | , 0 \leq h < s , \vec { b } _ { i \cdot s + h }$ models the difference of the $( h + 1 ) \cdot \operatorname { t h }$ bit $\mathrm { b i n } _ { h } ( \boldsymbol { x } _ { i } )$ of the $( i + 1 )$ -the entry $x _ { i }$ in the array ?? . Similarly, let $\vec { b }$ be the vector of Boolean variables such that for every $0 \leq i < | x | , 0 \leq h < s , \vec { b } _ { i \cdot s + h } ^ { \prime }$ models the difference of the $( h + 1 ) \cdot \mathrm { t h }$ bit $\mathsf { b i n } _ { h } ( y _ { i } )$ of the (?? + 1)-the entry $y _ { i }$ in the array $y . \mathrm { B y }$ expanding the matrix-vector product, we have: 

$$
M * x = \big (\bigoplus_ {j = 0} ^ {| x | - 1} (M _ {0, j} \otimes x _ {j}), \dots , \bigoplus_ {j = 0} ^ {| x | - 1} (M _ {| x | - 1, j} \otimes x _ {j}) \big).
$$

$y _ { i }$ $\textstyle \bigoplus _ { j = 0 } ^ { | x | - 1 } ( M _ { i , j } \otimes x _ { j } )$ $\vec { b } _ { i \cdot s + h } ^ { \prime }$ the $\left( h + 1 \right)$ -th bit $\mathsf { b i n } _ { h } ( y _ { i } )$ of the (?? + 1)-th entry $y _ { i }$ is the parity of the differences of the $\left( h + 1 \right)$ -th bits in $M _ { i , j } \otimes x _ { j }$ for $0 \leq j < h$ . 

Let $x ^ { \prime }$ be another input such that for every $0 \leq i < \vert x \vert , 0 \leq h < s , \vec { b } _ { i \cdot s + h } =  { \mathrm { b i n } } _ { h } ( x _ { i } ) \oplus  { \mathrm { b i n } } _ { h } ( x _ { i } ^ { \prime } )$ . Let $y ^ { \prime } = M * x ^ { \prime }$ . By expanding the finite-field multiplication (⊗) using a series of modular left shifts, we have that 

$$
\begin{array}{l} \vec {b} _ {i \cdot s + h} ^ {\prime} = \operatorname{bin} _ {h} \left(y _ {i}\right) \oplus \operatorname{bin} _ {h} \left(y _ {i} ^ {\prime}\right) \\ = \quad \operatorname{bin} _ {h} (\bigoplus_ {j = 0} ^ {| x | - 1} (M _ {i, j} \otimes x _ {j})) \oplus \operatorname{bin} _ {h} (\bigoplus_ {j = 0} ^ {| x | - 1} (M _ {i, j} \otimes x _ {j} ^ {\prime})) \\ = \quad \operatorname{bin} _ {h} (\bigoplus_ {j = 0} ^ {| x | - 1} ((M _ {i, j} \otimes x _ {j}) \oplus (M _ {i, j} \otimes x _ {j} ^ {\prime}))) \\ = \bigoplus_ {j = 0} ^ {| x | - 1} \operatorname{bin} _ {h} ((M _ {i, j} \otimes (x _ {j} \oplus x _ {j} ^ {\prime}))) \\ = \bigoplus_ {j = 0} ^ {| x | - 1} \operatorname{bin} _ {h} \left(\bigoplus_ {k = 0} ^ {s - 1} \operatorname{MSL} _ {\vec {c}} \left(\operatorname{bin} \left(x _ {j}\right) \oplus \operatorname{bin} \left(x _ {j} ^ {\prime}\right), M _ {i, j, k}, k\right)\right) \\ = \bigoplus_ {j = 0} ^ {| x | - 1} \operatorname{bin} _ {h} \left(\bigoplus_ {k = 0} ^ {s - 1} \operatorname{MSL} _ {\vec {c}} \left(\left(\vec {b} _ {j \cdot s + 0}, \dots , \vec {b} _ {j \cdot s + s - 1}\right), M _ {i, j, k}, k\right)\right) \\ \end{array}
$$

where the function ${ M S L } _ { \vec { c } }$ is defined as 

$$
\mathsf {M S L} _ {\vec {c}} (b _ {0}, \dots , b _ {s - 1}, b, k) = \left\{ \begin{array}{l l} (0, \dots , 0), & \text {if} b = 0; \\ (b _ {s - 1 - k}, b _ {s - k}, \dots , b _ {s - 1}, 0, \dots , 0) \oplus (\bigoplus_ {\iota = 0} ^ {s - 2 - k} b _ {\iota} ^ {j} \cdot \vec {c}), & \text {otherwise}. \end{array} \right.
$$

${ \vec { c } } = \left( { \vec { c } } _ { 0 } , \cdot \cdot \cdot , { \vec { c } } _ { s - 1 } \right) = \mathsf { b i n } ( 2 \otimes 2 ^ { s - 1 } )$ is the ??-bitstream corresponding to the coefficients of the irreducible polynomial for the underlying finite-field $( \mathrm { e . g . , } \vec { c } = \left( 0 , 0 , 0 , 1 , 1 , 0 , 1 , 1 \right)$ for $\mathbb { G F } ( 2 ^ { 8 } )$ whose irreducible polynomial is $X ^ { 8 } + X ^ { 4 } + X ^ { 3 } + X + 1$ in AES). 

Thus, $\vec { b } _ { i \cdot s + h } ^ { \prime }$ can be re-formulated as: 

$$
\begin{array}{l} \vec {b} _ {i \cdot s + h} ^ {\prime} = \bigoplus_ {j = 0} ^ {| x | - 1} \operatorname{bin} _ {h} \left(\bigoplus_ {k = 0} ^ {s - 1} \mathrm{MSL} _ {\vec {c}} \left(\left(\vec {b} _ {j \cdot s + 0}, \dots , \vec {b} _ {j \cdot s + s - 1}\right), M _ {i, j, k}, k\right)\right) \\ = \bigoplus_ {j = 0} ^ {| x | - 1} \left(\bigoplus_ {h \leq k <   s} \vec {b} _ {j \cdot s + k} \wedge M _ {i, j, k}\right) \oplus \left(\vec {c} _ {h} \wedge \bigoplus_ {0 \leq k <   \lfloor \frac {s}{2} \rfloor} \vec {b} _ {j \cdot s + 2 k}\right) \\ = \Big (\bigoplus_ {0 \leq j <   | x |, h \leq k <   s, M _ {i, j, k} = 1} \vec {b} _ {j \cdot s + k} \Big) \oplus \Big (\vec {c} _ {h} \wedge \bigoplus_ {0 \leq j <   | x |, 0 \leq k <   \lfloor \frac {s}{2} \rfloor} \vec {b} _ {j \cdot s + 2 k} \Big). \\ \end{array}
$$

Let $\{ b ^ { 0 } , \cdots , b ^ { m } \}$ be the set of support variables of the following formula 

$$
\Bigl (\bigoplus_ {0 \leq j <   | x |, h \leq k <   s, M _ {i, j, k} = 1} \vec {b} _ {j \cdot s + k} \Bigr) \oplus \Bigl (\vec {c} _ {h} \wedge \bigoplus_ {0 \leq j <   | x |, 0 \leq k <   \lfloor \frac {s}{2} \rfloor} \vec {b} _ {j \cdot s + 2 k} \Bigr).
$$

We have ${ \vec { b } } _ { i \cdot s + h } ^ { \prime } = \bigoplus _ { t = 0 } ^ { m } b _ { t }$ , which is equivalent to $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { v }$ for any $v \in \{ 1 , 2 , 3 \}$ . Thus, the result follows. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/a68c446d06c58e0a37594cf2f9f5d21baaf9b9db2cfb7307861874f63ca1cc62.jpg)


## D.4 Proof of Theorem 6.2

Theorem 6.2. Let $\ [ \boldsymbol { P } \ ] ^ { \mathsf { B } } = ( \Phi , \gamma , \Theta )$ and ?? be the minimum value of the objective function $\sum b \in \Theta$ ?? subject to the set of IL constraints $\Phi \cup \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \}$ . We have that $N \leq N _ { \mathsf { d i f f } }$ . 

Proof. The proof follows the lines of the proof of Theorem 5.2. Given a given program $P ,$ suppose $[ [ P ] ] ^ { \mathsf { B } } = ( \Phi , \gamma , \Theta )$ and ?? is the minimum value of the objective function $\sum _ { b \in \Theta } b$ subject to the set $\cup \left\{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \right\}$ $( K , X )$ $( K , X ^ { \prime } )$ that $X \neq X ^ { \prime }$ , let $\sigma _ { 0 } S _ { 1 } \sigma _ { 1 } S _ { 2 } \sigma _ { 2 } \cdot \cdot \cdot S _ { m } \sigma _ { m }$ and $\sigma _ { 0 } ^ { \prime } S _ { 1 } \sigma _ { 1 } ^ { \prime } S _ { 2 } \sigma _ { 2 } ^ { \prime } \cdot \cdot \cdot S _ { m } \sigma _ { m } ^ { \prime }$ be the sequences of states and statements of the executions under the inputs $( K , X )$ and $( K , X ^ { \prime } )$ , respectively. For each scalar variable ?? in a state $\sigma _ { i : }$ , let $b _ { x , j }$ be the Boolean variable modeling the difference of the $( j + 1 )$ -th most significant bit of bin(??) of ?? in Φ, and for each array variable ?? in a state $\sigma _ { i }$ , let $b _ { x , k , j }$ be the Boolean variable modeling the difference of the $( j + 1 )$ -th most significant bit of the $( k + 1 ) { \cdot } \mathrm { t h }$ entry $( \mathrm { i . e . , b i n } ( x _ { k } ) )$ of the array ?? in Φ. 

By Lemma 6.1 and induction on ?? and syntactic structure of statements and expressions, we can $b _ { x } = 1 ( \mathrm { r e s p . } b _ { x , j } = 1 )$ $\textstyle \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \}$ two executions $\sigma _ { 0 } S _ { 1 } \sigma _ { 1 } S _ { 2 } \sigma _ { 2 } \cdot \cdot \cdot S _ { m } \sigma _ { m }$ and $\sigma _ { 0 } ^ { \prime } S _ { 1 } \sigma _ { 1 } ^ { \prime } S _ { 2 } \sigma _ { 2 } ^ { \prime } \cdot \cdot \cdot S _ { m } \sigma _ { m } ^ { \prime }$ , the $( j + 1 )$ -th most significant bits of bin $\left( \sigma _ { i } ( x ) \right)$ and bin $\left( \sigma _ { i } ^ { \prime } ( x ) \right)$ (resp. $\left( k + 1 \right)$ -th entries of bin $\left( \sigma _ { i } ( x _ { k } ) \right)$ and bin $( \sigma _ { i } ^ { \prime } ( x _ { k } ) ) )$ are different. Thus, for any S-box S in the program ?? under those two executions, if the Boolean variable $b _ { S }$ that models the input difference of the S-box S is 1 in the solution of Φ $\cup \left\{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \right\}$ , then the S-box S must have different inputs, indicating that the S-box S is active. Therefore, we deduce that $N \leq N _ { \mathsf { d i f f } }$ . □ 

## D.5 Proof of Lemma 7.1

Lemma 7.1. Suppose $\mathbb { [ } e \mathbf { ] } \mathbb { ] } _ { \gamma } ^ { \mathsf { E B } } = ( \Psi , \vec { b } , \varrho )$ with $\Psi \ne \emptyset$ . The assignment $\{ b _ { 1 } , \cdot \cdot \cdot , b _ { m } , p _ { 1 } , \cdot \cdot \cdot , p _ { n } \}$ is a solution of Ψ iff the probability $o f \{ b _ { 1 } , \cdots , b _ { i } \}$ being bit-level differences of the operands and result of ?? is $2 ^ { - \varrho [ { p _ { 1 } } , \cdots , { p _ { n } } ] }$ , where $\varrho [ p _ { 1 } , \cdots , p _ { n } ]$ denotes the value of ?? under the assignment $\{ p _ { 1 } , \cdots , p _ { n } \}$ of the Boolean variables for encoding probabilities, and $\{ b _ { i + 1 } , \cdots , b _ { m } \}$ is the assignment of the auxiliary Boolean variables if exist. 

Proof. Suppose $\mathbb { [ } e \ b ] \ b { \mathrm { J } } _ { \gamma } ^ { \mathsf { E B } } = ( \Psi , \vec { b } , \varrho )$ . If ?? is of the form $\mathsf { V i e w } ( x , i , j ) , \sim x ,$ touint $\left( x _ { 1 } , \cdots , x _ { m } \right)$ or $x \langle \cdot y \cdot \rangle , \Psi = 0$ . If ?? is of the form $M * x \ \mathrm { o r } \ x _ { 1 } \oplus x _ { 2 }$ , by Lemma 6.1, $\{ b _ { 1 } , \cdots , b _ { m } \}$ satisfies Ψ iff the probability of $\{ b _ { 1 } , \cdots , b _ { i } \}$ being bit-level differences of the operands and result of ?? is 1. The result immediately follows from the fact that $\varrho = 0$ . If ?? is $x \langle y \rangle$ for S-box ??, the result follows from Proposition 4.4. Below, we consider the other cases. 

• ?? is $x _ { 1 } \odot x _ { 2 }$ for $\odot \in \{ \land , \lor \}$ . Suppose $\gamma ( x _ { 1 } ) = \vec { b } ^ { 1 }$ and $\gamma ( x _ { 2 } ) = { \vec { b } } ^ { 2 }$ , namely, $\vec { b } ^ { 1 }$ and $\vec { b } ^ { 2 }$ model the bit-level differences of the operands $x _ { 1 }$ and $x _ { 2 }$ respectively. Let $\vec { b } ^ { 0 }$ be the vector of Boolean variables each of which models the difference of a bit in the result of $x _ { 1 } \odot x _ { 2 }$ . 

For every $0 \leq i < \| x \|$ , for any fixed $\vec { b } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ , we observe that: 

$- \ \mathrm { i f } \ \vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0$ , then $\vec { b } _ { i } ^ { 0 } = 0 ;$ 

– otherwise $\vec { b } _ { i } ^ { 0 }$ could be 0 and 1 with the probability of $\begin{array} { l } { { \frac { 1 } { 2 } } } \end{array}$ 

Thus, for every $0 \leq i < \| x \|$ , 

– the probability of $\vec { b } _ { i } ^ { 0 } = 0 \mathrm { i } s 2 ^ { - 0 }$ when $\vec { b } _ { i } ^ { 1 } = \vec { b } _ { i } ^ { 2 } = 0$ , then $\vec { p } _ { i }$ must be 0. 

– the probability of $\vec { b } _ { i } ^ { 0 } = 1 \mathrm { i } s 2 ^ { - 1 }$ when $\vec { b } _ { i } ^ { 1 } + \vec { b } _ { i } ^ { 2 } \geq 1$ , then $\vec { p } _ { i }$ must be 1. 

The above two conditions are exactly characterized by Ψ??, i.e., $( \vec { b } _ { i } ^ { 1 } , \vec { b } _ { i } ^ { 2 } , \vec { b } _ { i } ^ { 0 } , \vec { p } _ { i } )$ is a solution of $\Psi _ { i }$ iff the probability of the difference $\vec { b } _ { i } ^ { 0 }$ of the (?? + 1)-th bit in the result of $x \odot y$ is $2 ^ { - { \vec { p } } _ { i } }$ when the differences of the (?? + 1)-th bits of the operands ?? and ?? are $ { \vec { b } } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ , respectively. Thus, the result immediately follows. 

• ?? is $x _ { 1 } \odot x _ { 2 }$ for $\odot \in \{ + , - \}$ . Suppose $\gamma ( x _ { 1 } ) = \vec { b } ^ { 1 }$ and $\gamma ( x _ { 2 } ) = { \vec { b } } ^ { 2 }$ , namely, $\vec { b } ^ { 1 }$ and $\vec { b ^ { 2 } }$ model the bit-level differences of the operands $x _ { 1 }$ and $x _ { 2 }$ respectively. Let $\vec { b } ^ { 0 }$ be the vector of Boolean variables each of which models the difference of a bit in the result of $x _ { 1 } + x _ { 2 }$ . 

Following the lines of the proof of Lemma 7.1, we have: 

$- \mathrm { \small ~ i f ~ } \vec { b } _ { i - 1 } ^ { 0 } = \vec { b } _ { i - 1 } ^ { 1 } = \vec { b } _ { i - 1 } ^ { 2 }$ , then $\vec { p } _ { i }$ should be $0 ~ ( \mathrm { i . e . }$ , the probability $2 ^ { - { \vec { p } } _ { i } }$ of $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ or $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } \mathrm { ~ i s ~ } 1 )$ , 

$\begin{array} { r } { - \mathrm { ~ i f ~ } 1 \le \sum _ { j = 0 } ^ { 2 } \vec { b } _ { i - 1 } ^ { j } \le 2 , } \end{array}$ , then $\vec { p } _ { i }$ should be 1 (i.e., the probability $2 ^ { - { \vec { p } } _ { i } }$ of $\vec { b } _ { i } ^ { 0 } = 1 \ \mathrm { o r } \ \vec { b } _ { i } ^ { 0 } = 0 \ \mathrm { i s } \ \frac { 1 } { 2 } )$ 

The above two conditions are exactly characterized by $\Psi _ { i } ^ { 1 } \cup \Psi _ { i } ^ { 2 }$ , where $\Psi _ { i } ^ { 1 }$ ensures that $\vec { p } _ { i } = 0$ if $\vec { b } _ { i - 1 } ^ { 0 } = \vec { b } _ { i - 1 } ^ { 1 } = \vec { b } _ { i - 1 } ^ { 2 }$ , and $\begin{array} { r } { \vec { p } _ { i } = 1 \mathrm { ~ i f ~ } 1 \le \sum _ { j = 0 } ^ { 2 } \vec { b } _ { i - 1 } ^ { j } \le 2 ; \Psi _ { i } ^ { 2 } } \end{array}$ ensures that $\vec { b } _ { i } ^ { 0 } = \neg ( \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } )$ if $\begin{array} { r } { \sum _ { j = 0 } ^ { 2 } { \vec { b } } _ { i - 1 } ^ { j } = 3 } \end{array}$ $\vec { b } _ { i } ^ { 0 } = \vec { b } _ { i } ^ { 1 } \oplus \vec { b } _ { i } ^ { 2 } \mathrm { i f } \sum _ { j = 0 } ^ { 2 } \vec { b } _ { i - 1 } ^ { j } = 0$ $( \vec { b } ^ { 0 } , \vec { b } ^ { 1 } , \vec { b } ^ { 2 } , \vec { p } )$ $\textstyle \bigcup _ { i = 0 } ^ { \left\| x \right\| - 1 } \Psi _ { i }$ ${ \vec { b } } ^ { 0 }$ of the modular addition (+) for the given input differences $\vec { b } ^ { 1 }$ and ${ \vec { b } } ^ { 2 } \operatorname { i s } 2 ^ { - \sum _ { i = 0 } ^ { \| x \| - 1 } } { \vec { p } } _ { i }$ . 

For $x _ { 1 } - x _ { 2 }$ , it is easy to deduce that $[ [ [ [ \boldsymbol { x } _ { 1 } - \boldsymbol { x } _ { 2 } ] ] _ { Y } ^ { \mathsf { E B } } = [ [ \boldsymbol { x } _ { 1 } + \boldsymbol { x } _ { 2 } ] ] _ { Y } ^ { \mathsf { E B } } $ , because if $y = x _ { 1 } - x _ { 2 }$ , then $x _ { 2 } = x _ { 1 } + y$ , and the Boolean variables $\vec { b } _ { i } ^ { 0 } , \vec { b } _ { i } ^ { 1 }$ and $\vec { b } _ { i } ^ { 2 }$ in all the IL constraints of $\left[ \left[ x _ { 1 } + x _ { 2 } \right] \right] _ { \gamma } ^ { \mathsf { E B } }$ are symmetric except for $\vec { p } _ { i } \geq \vec { b } _ { i - 1 } ^ { 0 } - \vec { b } _ { i - 1 } ^ { 1 } , \vec { p } _ { i } \geq \vec { b } _ { i - 1 } ^ { 1 } - \vec { b } _ { i - 1 } ^ { 2 }$ and $\vec { p } _ { i } \geq \vec { b } _ { i - 1 } ^ { 2 } - \vec { b } _ { i - 1 } ^ { 0 }$ which still work in $\left[ \left[ x _ { 1 } - x _ { 2 } \right] \right] _ { \gamma } ^ { \mathsf { E B } }$ . 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/3b8a9c15215e35b9e5490aab524a6e08f2769850d3dc5713df89759b7c8cfe0d.jpg)


## D.6 Proof of Theorem 7.2

Theorem 7.2. Let $\ [ P \ ] ^ { \mathsf { E B } } = ( \Phi , \gamma , \varrho )$ and ?? be the minimum value of the objective function ?? subject to $\begin{array} { r } { \Phi \cup \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \} } \end{array}$ of the program ?? is no greater than $2 ^ { - u }$ . 

Proof. Suppose $\ [ P \ ] ^ { \mathsf { E B } } = ( \Phi , \gamma , \varrho )$ for the given program ??. For any solution $\{ b _ { 1 } , \cdot \cdot \cdot , b _ { m } , p _ { 1 } , \cdot \cdot \cdot , p _ { n } \}$ of $\Phi \cup \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \}$ that corresponds an ??-round differential characteristic $( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ , by Lemma 7.1, the differential characteristic probability $\mathsf { P r } _ { \mathsf { E n c } } ( \Delta X ^ { 0 } , \cdot \cdot \cdot , \Delta X ^ { s } )$ is no greater than $2 ^ { - } \textstyle \sum _ { i = 0 } ^ { n } p _ { i }$ . 

Let ?? be the minimum value of the objective function ?? subject to the set of IL constraints $\Phi \cup \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \}$ . Clearly, the differential characteristic probability $\mathsf { P r } _ { \mathsf { E n c } } ( \widetilde { \Delta X } ^ { 0 } , \cdots , \widetilde { \Delta X } ^ { s } )$ of any optimal ??-round differential characteristic $( \widetilde { \Delta X } ^ { 0 } , \cdot \cdot \cdot , \widetilde { \Delta X } ^ { s } )$ is no greater than $2 ^ { u }$ . □ 

## E FURTHER EXPERIMENTAL RESULTS

In this section, we compare the performance of various alternative methods to generate MILP. In summary, we find that (below, Ψ with super/subscripts refer to IL constraints specified in the semantic rules from Sections $4 { - } 7 )$ : 

(1) $\Psi _ { 2 , 3 } ^ { 1 }$ performs much better than $\Psi _ { 2 , 3 } ^ { 2 }$ , though 3 out of 4 constraints in $\Psi _ { 2 , 3 } ^ { 2 }$ are much simpler; 

(2) $\Psi _ { M } ^ { 1 }$ and $\Psi _ { M } ^ { 2 }$ are almost comparable though $\Psi _ { M } ^ { 1 }$ contains significantly fewer constraints; 

(3) removing the redundant constraints $\Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 2 }$ and/or $\Psi _ { S } ^ { \mathsf { b n } }$ from the (extended) bit-wise modeling of S-boxes often significantly degrades the performance; 

(4) $\Psi _ { \oplus } ^ { 2 }$ performs much better than $\Psi _ { \oplus } ^ { 1 }$ and $\Psi _ { \oplus } ^ { 3 }$ though $\Psi _ { \oplus } ^ { 2 }$ contains more constraints than $\Psi _ { \oplus } ^ { 3 }$ 

(5) $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 2 }$ outperforms $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 1 }$ $\Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 3 }$ $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 2 }$ $\Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 3 } ;$ 


Table 11. Results of the word-wise approach, where #AS denotes the minimum number of active S-boxes.


<table><tr><td colspan="2">Rounds</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td><td>16</td><td>17</td><td>18</td><td>19</td><td>20</td><td>21</td><td>22</td><td>23</td><td>24</td><td>25</td><td>26</td><td>27</td><td>28</td><td>29</td><td>30</td><td>31</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td></tr><tr><td rowspan="2">AES</td><td>#AS</td><td>1</td><td>5</td><td>9</td><td>25</td><td>26</td><td>30</td><td>34</td><td>50</td><td>51</td><td>55</td><td>59</td><td>75</td><td>76</td><td>80</td><td colspan="11"></td><td colspan="10">N/A</td><td></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td colspan="11"></td><td colspan="10">N/A</td><td></td></tr><tr><td rowspan="2">KLEIN</td><td>#AS</td><td>1</td><td>5</td><td>8</td><td>15</td><td>16</td><td>20</td><td>23</td><td>30</td><td>31</td><td>35</td><td>38</td><td>45</td><td colspan="13"></td><td colspan="10">N/A</td><td></td></tr><tr><td>Time</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td colspan="13"></td><td colspan="10">N/A</td><td></td></tr><tr><td rowspan="2">LBLOCK</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>6</td><td>8</td><td>11</td><td>14</td><td>18</td><td>22</td><td>24</td><td>27</td><td>30</td><td>32</td><td>35</td><td>36</td><td>39</td><td>41</td><td>44</td><td>45</td><td>48</td><td>50</td><td>53</td><td>54</td><td>57</td><td>59</td><td>62</td><td>63</td><td>66</td><td>68</td><td>71</td><td></td><td>N/A</td><td></td><td></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>1s</td><td>1s</td><td>3s</td><td>3s</td><td>2s</td><td>6s</td><td>5s</td><td>62s</td><td>62s</td><td>80s</td><td>385s</td><td>273s</td><td>492s</td><td>359s</td><td>1036s</td><td>3607s</td><td>8312s</td><td></td><td>N/A</td><td></td><td></td></tr><tr><td rowspan="2">MIBS</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>5</td><td>6</td><td>7</td><td>8</td><td>11</td><td>12</td><td>13</td><td>14</td><td>17</td><td>18</td><td>19</td><td>20</td><td>23</td><td>24</td><td>25</td><td>26</td><td>29</td><td>30</td><td>31</td><td>32</td><td>35</td><td>36</td><td>37</td><td>38</td><td>41</td><td>42</td><td>43</td><td>44</td><td>47</td><td></td><td>N/A</td><td></td><td></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>1s</td><td>1s</td><td>1s</td><td>3s</td><td>4s</td><td>2s</td><td>4s</td><td>2s</td><td>3s</td><td>2s</td><td>3s</td><td>4s</td><td>5s</td><td>4s</td><td>7s</td><td>4s</td><td>5s</td><td>9s</td><td>11s</td><td>9s</td><td>8s</td><td>20s</td><td></td><td>N/A</td><td></td><td></td></tr><tr><td rowspan="2">PHOTON</td><td>#AS</td><td>1</td><td>9</td><td>17</td><td>81</td><td>82</td><td>90</td><td>98</td><td>162</td><td>163</td><td>171</td><td>179</td><td>243</td><td colspan="13"></td><td colspan="10">N/A</td><td></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td colspan="13"></td><td colspan="10">N/A</td><td></td></tr><tr><td rowspan="2">PICCOLO</td><td>#AS</td><td>0</td><td>5</td><td>10</td><td>15</td><td>20</td><td>30</td><td>35</td><td>40</td><td>45</td><td>50</td><td>55</td><td>60</td><td>65</td><td>70</td><td>75</td><td>80</td><td>85</td><td>90</td><td>95</td><td>100</td><td>105</td><td>110</td><td>115</td><td>120</td><td>125</td><td colspan="4"></td><td colspan="6">N/A</td><td></td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>3s</td><td>6s</td><td>4s</td><td>6s</td><td>28s</td><td>30s</td><td>31s</td><td>38s</td><td>187s</td><td>93s</td><td>122s</td><td>289s</td><td>560s</td><td colspan="4"></td><td colspan="6">N/A</td><td></td></tr><tr><td rowspan="2">TWINE</td><td>#AS</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>6</td><td>8</td><td>11</td><td>14</td><td>18</td><td>22</td><td>24</td><td>27</td><td>30</td><td>32</td><td>35</td><td>36</td><td>39</td><td>41</td><td>44</td><td>45</td><td>48</td><td>50</td><td>53</td><td>54</td><td>57</td><td>59</td><td>62</td><td>63</td><td>66</td><td>68</td><td>71</td><td>72</td><td>75</td><td>77</td><td>80</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>0s</td><td>0s</td><td>1s</td><td>0s</td><td>1s</td><td>2s</td><td>2s</td><td>3s</td><td>2s</td><td>5s</td><td>6s</td><td>26s</td><td>79s</td><td>72s</td><td>196s</td><td>166s</td><td>173s</td><td>786s</td><td>858s</td><td>5395s</td><td>6669s</td><td>8168s</td><td>26348s</td><td>19771s</td><td>18743s</td></tr></table>

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/3339dc74d36ac27ac017d8bf9d5714beb93ada5bf513e46ad33650c63720937e.jpg)



(a) LBLOCK


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/0dfb97346e7dfb63a3e0fb3d4b00539413733acafd22f34ea96a692a767a5693.jpg)



(b) TWINE


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/1ab175bc44ae58ffc6dc115a8ad8b6d995758ca0edd95123195b8f0565a8b34d.jpg)



(c) PICCOLO



Fig. 11. Comparison of alternative modeling methods in the word-wise approach.


$\Psi _ { S } ^ { 3 }$ produce the fewest constraints with the same number of Boolean variables, but exhibit the least efficiency in determining the lower bound of the minimum number of active S-boxes, instead, the techniques of [Boura and Coggia 2020] in general outperform the others; 

(7) the techniques of [Sasaki and Todo 2017; Sun et al. 2014b] are comparable for constructing $\Psi _ { S } ^ { 4 }$ others including [Boura and Coggia 2020; Li and Sun 2022; Udovenko 2021]. 

## E.1 More Results of the Word-wise Approach

The minimum number of active S-boxes of the entire rounds of all the word-wise implementations are reported in Table 11. 

## E.1.1 Comparison of Alternative Word-wise Modeling Methods. In our word-wise approach (cf. Figure 6),

$\Psi _ { 2 , 3 } ^ { 1 }$ $\Psi _ { 2 , 3 } ^ { 2 }$ $( \mathbf { e . g . , + , - , \oplus } )$ whose minimum and maximum word-wise branch numbers are 2 and 3, respectively; 

• $\Psi _ { M } ^ { 1 }$ and $\Psi _ { M } ^ { 2 }$ are two alternative methods for modeling the matrix-vector product ?? ∗ ??. 

$\Psi _ { 2 , 3 } ^ { 1 } \Psi \mathbf { s } . \Psi _ { 2 , 3 } ^ { 2 }$ use the XOR operator. The results are depicted in Figures 11a and 11b, respectively, where the number on top of the bar is the execution time in seconds (s). Note that the results for small rounds $\Psi _ { 2 , 3 } ^ { 1 }$ $\Psi _ { 2 , 3 } ^ { 2 }$ , in $\Psi _ { 2 , 3 } ^ { 1 }$ although 3 out of 4 constraints in $\Psi _ { 2 , 3 } ^ { 2 }$ are much simple. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/1cc85ce7cd4b9081c51475c9dab6cb9aba3712a76399156aea690e090f7c1fd4.jpg)



(a) PRESENT


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/b2fb9db98c6a7f8d343467595439a53c1c1b807357afe4d4099d92e40e9ddac6.jpg)



(b) SKINNY


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/94381ed8777666f09da83beed5682e7c1f594d421f8f328ad3fb02504242ebcb.jpg)



(c) LBLOCK


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/35e7cf46826f3a22a1b3d2ce32f80ae40f58e1e5a8c6489eb56f35c7769acb07.jpg)



(d) KLEIN


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/18f042b5e37029dd2a7d9e1df034eaca71212e589ee59ea6ee6326f6edda7927.jpg)



(e) SKINNY



Fig. 12. Comparison of alternative modeling methods in the bit-wise approach.


$\Psi _ { M } ^ { 1 }$ $\Psi _ { M } ^ { 2 }$ (up to 25 rounds) that use the matrix-vector product, where $\Psi _ { 2 , 3 } ^ { 1 }$ is used if the XOR operator is used. $\Psi _ { M } ^ { 1 }$ $\Psi _ { M } ^ { 2 }$ reported. The results of PICCOLO are depicted in Figure 11c. We found that $\Psi _ { M } ^ { 1 }$ performs better than $\Psi _ { M } ^ { 2 }$ for some rounds (e.g., 21, 22, 23), but worse than $\Psi _ { M } ^ { 2 }$ for some other rounds (e.g., 20, 24, 25). $\Psi _ { M } ^ { 1 }$ $\Psi _ { M } ^ { 2 }$ fewer constraints do not always yield better performance. Note that the results of the combinations $\Psi _ { 2 , 3 } ^ { 1 }$ $\Psi _ { 2 , 3 } ^ { 2 }$ and $\Psi _ { M } ^ { 1 }$ $\Psi _ { M } ^ { 2 }$ 

## E.2 Comparison of Alternative Bit-wise Modeling Methods

In our bit-wise approach (cf. Figure 8), 

• $\psi _ { \oplus } ^ { 1 } , \psi _ { \oplus } ^ { 2 }$ and $\psi _ { \oplus } ^ { 3 }$ are three alternative methods for modeling the ⊕ operator; 

Ψ1 $\Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 1 } , \Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 2 }$ and Ψ3 $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 3 }$ are three alternative methods for modeling $M * x ;$ ; 

• $\Psi _ { S } \cup \Psi _ { S } ^ { \mathsf { b n } }$ for modeling S-boxes can be safely simplified to $\Psi _ { S }$ and $\Psi _ { S } ^ { 3 }$ . Recall that $\Psi _ { S } = $ $\Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 2 } \cup \Psi _ { S } ^ { 3 }$ $s$ $\Psi _ { S } = \Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 3 }$ $\Psi _ { S } ^ { 1 } , \Psi _ { S } ^ { 2 }$ $\Psi _ { S } ^ { \mathsf { b n } }$ are redundant. 

• $\Psi _ { S } ^ { 3 }$ can be constructed from the DDT $\mathcal { D } _ { S }$ of an S-box S using different techniques. 

Results of $\Psi _ { S } \cup \Psi _ { S } ^ { \mathsf { b n } }$ vs. $\Psi _ { S }$ vs. $\Psi _ { S } ^ { 3 }$ . We conduct experiments on PRESENT (up to 13 rounds) and SKINNY (up to 13 rounds) that use S-boxes where $\Psi _ { S } ^ { 3 }$ is constructed using the technique from [Sun $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 2 }$ in Figure 12a and Figure 12b, where timeout (TO) is set to 1.5 times of the best one for the 13 round $( \mathrm { i . e . , 5 3 6 3 0 s \times 1 . 5 ) }$ . The results for small rounds are negligible, thus are omitted here. We can $\Psi _ { S } \cup \Psi _ { S } ^ { \mathsf { b n } }$ $\Psi _ { S }$ and $\Psi _ { S } ^ { 3 }$ . It seems that more constraints provide more information to improve the efficiency of MILP solving. 

$\Psi _ { \oplus } ^ { 1 }$ $\Psi _ { \oplus } ^ { 2 }$ vs. $\Psi _ { \oplus } ^ { 3 }$ . We conduct experiments on LBLOCK (up to 13 rounds) and KLEIN $\Psi _ { S } \cup \Psi _ { S } ^ { \mathrm { b n } }$ from [Sun et al. 2014b] are used. The results are depicted in Figure 12c and Figure 12d, where timeout is set the same as above. We can observe that $\Psi _ { \oplus } ^ { 2 }$ achieves the best performance, although it contains more IL constraints than $\Psi _ { \oplus } ^ { 3 }$ . It may be because $\Psi _ { \oplus } ^ { 2 }$ does not introduce additional variables, while $\Psi _ { \oplus } ^ { 1 }$ and $\Psi _ { \oplus } ^ { 3 }$ introduce one additional Boolean variable for one bit. 


Table 12. Comparison of techniques for constructing $\Psi _ { S } ^ { 3 }$ of individual S-boxes


<table><tr><td rowspan="2">Size</td><td rowspan="2">S-box</td><td colspan="2"><eq>T_1</eq></td><td colspan="2"><eq>T_2</eq></td><td colspan="2"><eq>T_3</eq></td><td colspan="2"><eq>T_4</eq></td><td colspan="2"><eq>T_5</eq></td><td colspan="2"><eq>T_6</eq></td><td colspan="2"><eq>T_7</eq></td><td colspan="2"><eq>T_8</eq></td></tr><tr><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td><td><eq>|\Psi_S^3|</eq></td><td>Time</td></tr><tr><td rowspan="17">4-bit</td><td>Elephant</td><td>26</td><td>0s</td><td>23</td><td>1s</td><td>40</td><td>0s</td><td>19</td><td>149s</td><td>38</td><td>0s</td><td>19</td><td>3s</td><td>17</td><td>5s</td><td>17</td><td>0s</td></tr><tr><td>GIFT</td><td>25</td><td>0s</td><td>21</td><td>0s</td><td>34</td><td>0s</td><td>17</td><td>176s</td><td>33</td><td>2s</td><td>18</td><td>7s</td><td>17</td><td>30s</td><td>16</td><td>0s</td></tr><tr><td>LBLOCK S0</td><td>27</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>108s</td><td>30</td><td>1s</td><td>17</td><td>6s</td><td>15</td><td>30s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S1</td><td>26</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>107s</td><td>30</td><td>2s</td><td>17</td><td>9s</td><td>15</td><td>31s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S2</td><td>27</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>131s</td><td>30</td><td>3s</td><td>17</td><td>10s</td><td>15</td><td>30s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S3</td><td>27</td><td>0s</td><td>24</td><td>1s</td><td>31</td><td>0s</td><td>17</td><td>143s</td><td>30</td><td>2s</td><td>17</td><td>10s</td><td>15</td><td>28s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S4</td><td>27</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>144s</td><td>30</td><td>3s</td><td>17</td><td>10s</td><td>15</td><td>30s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S5</td><td>29</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>143s</td><td>30</td><td>2s</td><td>17</td><td>10s</td><td>15</td><td>32s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S6</td><td>26</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>140s</td><td>30</td><td>2s</td><td>17</td><td>10s</td><td>15</td><td>31s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S7</td><td>26</td><td>0s</td><td>24</td><td>0s</td><td>30</td><td>0s</td><td>17</td><td>140s</td><td>30</td><td>2s</td><td>17</td><td>10s</td><td>15</td><td>32s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S8</td><td>26</td><td>0s</td><td>24</td><td>1s</td><td>31</td><td>0s</td><td>17</td><td>139s</td><td>30</td><td>2s</td><td>17</td><td>9s</td><td>15</td><td>30s</td><td>15</td><td>0s</td></tr><tr><td>LBLOCK S9</td><td>26</td><td>0s</td><td>24</td><td>0s</td><td>31</td><td>0s</td><td>17</td><td>133s</td><td>30</td><td>1s</td><td>17</td><td>6s</td><td>15</td><td>29s</td><td>15</td><td>0s</td></tr><tr><td>PICCOLO</td><td>24</td><td>0s</td><td>21</td><td>0s</td><td>31</td><td>0s</td><td>16</td><td>133s</td><td>31</td><td>2s</td><td>16</td><td>8s</td><td>14</td><td>23s</td><td>14</td><td>0s</td></tr><tr><td>PRESENT</td><td>24</td><td>0s</td><td>21</td><td>0s</td><td>39</td><td>0s</td><td>17</td><td>274s</td><td>36</td><td>1s</td><td>17</td><td>5s</td><td>17</td><td>19s</td><td>16</td><td>0s</td></tr><tr><td>RECTANGLE</td><td>23</td><td>0s</td><td>21</td><td>0s</td><td>31</td><td>0s</td><td>17</td><td>246s</td><td>30</td><td>2s</td><td>17</td><td>9s</td><td>15</td><td>34s</td><td>15</td><td>0s</td></tr><tr><td>SKINNY</td><td>24</td><td>0s</td><td>21</td><td>0s</td><td>31</td><td>0s</td><td>16</td><td>135s</td><td>31</td><td>2s</td><td>16</td><td>7s</td><td>14</td><td>35s</td><td>14</td><td>0s</td></tr><tr><td>TWINE</td><td>27</td><td>0s</td><td>23</td><td>0s</td><td>47</td><td>0s</td><td>20</td><td>291s</td><td>45</td><td>2s</td><td>19</td><td>5s</td><td>19</td><td>7s</td><td>19</td><td>0s</td></tr><tr><td>5-bit</td><td>ASCON</td><td>50</td><td>3s</td><td>40</td><td>19s</td><td>60</td><td>0s</td><td>31</td><td>267535s</td><td>59</td><td>130s</td><td>48</td><td>14776s</td><td>28</td><td>2022s</td><td>27</td><td>72s</td></tr></table>

$\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 1 }$ vs. $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 2 }$ $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 3 }$ We conduct experiments on SKINNY (up to 9 rounds) that uses the matrix-vector product and S-boxes for which $\Psi _ { S } \cup \Psi _ { S } ^ { \mathrm { b n } }$ and the technique from [Sun et al. 2014b] are used. The results are depicted in Figure 12e, where timeout is set the same as above. We $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 2 }$ $\Psi _ { \oplus } ^ { 1 } { : }$ $\Psi _ { \oplus } ^ { 2 }$ $\Psi _ { \oplus } ^ { 3 }$ $\Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 1 } , \Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 2 }$ $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 3 }$ $\Psi _ { \mathsf { M } , \mathsf { i } , \mathsf { h } } ^ { 3 }$ fewer constraints, but achieves the worst performance. 

Results of techniques for constructing $\Psi _ { S } ^ { 3 }$ . The techniques are named as follows for simplifying presentation: $\mathbf { T } _ { 1 } { = } [ \mathrm { S u n \ e t \ a l . \ } 2 0 1 4 \mathrm { b } ]$ , $\mathbf { T } _ { 2 } \mathbf { = } [ \boldsymbol { \mathrm { S } }$ asaki and Todo 2017], $\mathbf { T } _ { 3 } { = } [ \mathbf { \Phi } _ { }$ Abdelkhalek et al. 2017], $\mathbf { T } _ { 4 } { = } \mathrm { A l g } . 1$ 1 of [Boura and Coggia 2020], $\mathrm { \mathbf { T } } _ { 5 } { = } \mathrm { \mathbf { A l g } } . 2$ of [Boura and Coggia 2020], $\mathbf { T } _ { 6 } { = } \mathrm { A l g } . 2 , \mathrm { A l g } . 3$ and Proposition 3 of [Boura and Coggia 2020], $\mathbf { T } _ { 7 } \mathbf { = } [ \mathrm { L i }$ and Sun 2022], and $\mathbf { T } _ { 8 } { = } [$ [Udovenko 2021]. 

We first compare their performance for constructing $\Psi _ { S } ^ { 3 }$ of 4-bit and 5-bit S-boxes from randomly chosen ciphers in terms of number of constraints and execution time. The results are reported in Table 12. We observe that 

• $\mathbf { T } _ { 8 }$ can efficiently produce the fewest constraints with the same number of Boolean variables, 

• ${ \bf T } _ { 3 }$ is the most efficient but produces the most constraints, 

• $\mathbf { T } _ { 4 }$ takes the longest time. Note that the constraint coefficients produced by ${ \bf T } _ { 3 }$ are limited to $\{ - 1 , 0 , 1 \}$ while the others do not. 

We compare the performance for the overall security analysis in terms of execution time on randomly chosen ciphers using $\psi _ { \oplus } ^ { 2 } , \Psi _ { S } \cup \Psi _ { S } ^ { \mathrm { b n } }$ and $\Psi _ { \mathsf { M } , \mathrm { i } , \mathsf { h } } ^ { 2 } .$ execution time of constructing $\Psi _ { S } ^ { 3 }$ is excluded as it is only computed once for each S-box. Overall, the performance varies with ciphers and round numbers, and no technique always outperforms the others. Interestingly, we found that 

• though the recent promising techniques $\mathbf { T } _ { 7 }$ and $\mathbf { T } _ { 8 }$ produce the fewest constraints, they often performs worse than the others (e.g., on LBLOCK, PICCOLO and TWINE); 

• though ${ \bf T } _ { 3 }$ produces the most constraints, it performs moderately on all the ciphers, because the generated constraints are much simpler; 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/468f88ee761ca9ea22abddbc6f8e231808a5bd296af37a167029481adebf407c.jpg)



(a) PRESENT


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/586c47a335f0f9e5ea007d8238df47fa7b65395b392f01da0cb045fb493e6d75.jpg)



(b) SKINNY


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/2088c12d229fa443f3f772f4a84930e1ab1ebbf3230952e3b50f5d57c24d7850.jpg)



(c) LBLOCK


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/b629e72d4d90027c98e11f584b68ace396119fb55e6460d2bb8f6c54434fbcf4.jpg)



(d) PICCOLO


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/267e9513a18e0ee7a9f0609978eabbb9dfb9c64a4a719949ec9077fc70b62922.jpg)



(e) TWINE


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/800f6e8e389ab4f70ec7f0829c4068a0e1188cd45544f358d48b6d109bf7a487.jpg)



(f) ASCON



Fig. 13. Comparison of techniques for constructing $\Psi _ { S } ^ { 3 }$ in the overall security analysis.


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/6a3257429f12f2c2a393c151bedbc80f74495ae1a8150b873adf5ac16d438c29.jpg)



(a) PRESENT


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/50261d137fdf66c3a7995efb606480ba916530ee801c8f16ab5639a9b3b4c7d8.jpg)



(b) TWINE


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/16e43bd2850d2d5b408c8cbe6961d846a236fa7ac863c0e32b4a42bda4009e13.jpg)



(c) SKINNY



Fig. 14. Comparison of S-boxes modeling methods in the extended bit-wise approach.



Table 13. Comparison of techniques for constructing $\Psi _ { S } ^ { 4 }$ of individual S-boxes, where timeout (TO) is 1 hour.


<table><tr><td rowspan="2">S-box</td><td colspan="2"><eq>T_1</eq></td><td colspan="2"><eq>T_2</eq></td><td colspan="2"><eq>T_3</eq></td><td colspan="2"><eq>T_4</eq></td><td colspan="2"><eq>T_5</eq></td><td colspan="2"><eq>T_6</eq></td><td colspan="2"><eq>T_7</eq></td><td colspan="2"><eq>T_8</eq></td></tr><tr><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td><td><eq>|Ψ_S^4|</eq></td><td>Time</td></tr><tr><td>GIFT</td><td>27</td><td>1s</td><td>19</td><td>3s</td><td>50</td><td>0s</td><td colspan="2">TO</td><td colspan="2">TO</td><td colspan="2">TO</td><td colspan="2">TO</td><td colspan="2">N/A</td></tr></table>

• though $\mathbf { T } _ { 4 }$ and $\mathbf { T } _ { 6 }$ take the longest time to construct $\Psi _ { S } ^ { 3 }$ and do not produce the fewest constraints, they in general perform well except for PRESENT with 12 rounds and PICCOLO with 5 rounds respectively. 

In summary, not only the number but also coefficients of the constraints affect the overall efficiency. It is interesting to study the characterizations of optimal constraints in future. 

## E.3 Comparison of Alternative Extended Bit-wise Modeling Methods

In our extended bit-wise approach, there are also three alternatives for modeling S-boxes, namely, $\Psi _ { S } ^ { \dagger } \cup \Psi _ { S } ^ { \mathsf { b n } } , \Psi _ { S } ^ { \dagger }$ ∪ Ψ bnS , Ψ †S and $\Psi _ { S } ^ { 4 }$ . Note that $\Psi _ { S } ^ { \dagger } = \Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 2 } \cup \Psi _ { S } ^ { 4 }$ if the S-box S is injective, otherwise $\Psi _ { S } ^ { \dagger } = \Psi _ { S } ^ { 1 } \cup \Psi _ { S } ^ { 4 }$ , where $\Psi _ { S } ^ { 1 } , \Psi _ { S } ^ { 2 }$ and $\Psi _ { S } ^ { \flat \ n }$ are redundant. 

$\Psi _ { S } ^ { \dagger } \cup \Psi _ { S } ^ { \mathsf { b n } }$ $\Psi _ { S } ^ { \dagger }$ vs. $\Psi _ { S } ^ { 4 }$ . We conduct experiments on PRESENT (up to 11 rounds), $\Psi _ { S } ^ { 4 }$ from [Sun et al. 2014b]. The results are given in Figure 14a, Figure 14b and Figure 14c, where timeout is set the same as above, i.e., 1.5 times of the best one for the largest round number. We can observe $\Psi _ { S } ^ { \dagger } \cup \Psi _ { S } ^ { \mathsf { b n } }$ $\Psi _ { S } ^ { 4 }$ $\Psi _ { S } ^ { \dagger } \cup \Psi _ { S } ^ { \mathsf { b n } }$ $\Psi _ { S } ^ { \dagger } \cup \Psi _ { S } ^ { \mathsf { b n } }$ achieves the best performance. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/d42e5d06f8867e1ac09abd534f738ce8eea47ccf647bb01840f2062c7d31a2ba.jpg)



(a) GIFT-COFB


![image](https://cdn-mineru.openxlab.org.cn/result/2026-06-11/79fec95c-76e4-44c4-9bcf-49bc9272c5a1/83dcc932dc63e5f54754e05b4229ecb76b0a9d4724a8e4a6dc1e632ade6b9730.jpg)



(b) GIFT



Fig. 15. Comparison of techniques for constructing $\Psi _ { S } ^ { 4 }$ in the overall security analysis.


Results of techniques for constructing $\Psi _ { S } ^ { 4 }$ We compare the techniques $\mathbf { T } _ { i } , 1 \le i \le 8 .$ for constructing $\Psi _ { S } ^ { 4 }$ using GIFT-COFB and GIFT. Note that GIFT-COFB and GIFT use the same S-box. 

The comparison of the techniques $\mathbf { T } _ { i } , 1 \le i \le 8$ for constructing $\Psi _ { S } ^ { 4 }$ of GIFT-COFB (GIFT) in terms of number of constraints and execution time are given in Table 13, where timeout (TO) is set to 1 hour. We can observe that 

• $\mathbf { T } _ { 2 }$ produces the fewest constraints, 

• ${ \bf T } _ { 3 }$ is the most efficient, 

• $\mathbf { T } _ { 4 } , \mathbf { T } _ { 5 } , \mathbf { T } _ { 6 }$ and $\mathbf { T } _ { 7 }$ all run out of time, 

• ${ \bf T } _ { 8 }$ fails due to bugs. 

Note that we use the open-source implementations of $\mathbf { T } _ { 7 }$ and $\mathbf { T } _ { 8 }$ provided by their authors and the bugs have been reported to the author of ${ \bf T } _ { 8 }$ . 

The comparison of the techniques for the overall security analysis in terms of execution time are reported in Figure 15a and Figure 15b respectively, using $\Psi _ { S } ^ { \dagger } \cup \Psi _ { S } ^ { \mathsf { b n } }$ . We found that ${ \bf T } _ { 1 }$ and $\mathbf { T } _ { 2 }$ are comparable, and performs better than $\mathbf { T } _ { 3 } ,$ , probably because ${ \bf T } _ { 3 }$ produces too many constraints. 

## E.4 Bounding Condition

Inspired by [Zhang et al. 2018], we formalize the bounding condition of [Matsui 1994] for the bit-wise approach, where the formalizations for the word-wise approach and the extended bit-wise approach can be defined similarly. 

Suppose we are going to determine the lower bound of the minimum number of active S-boxes of ?? -round differential characteristics. We will add the following additional constraints into the set of IL constraints $\Phi \cup \{ \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \geq 1 \}$ that are constructed by our bit-wise approach (cf. Section 6): 

$\begin{array} { r } { 1 \leq i \leq r , \sum _ { j = 0 } ^ { s \cdot n - 1 } \gamma ( x _ { i } , j ) \geq 1 } \end{array}$ $x _ { i }$ by the return of the ??-th round function, and its type is uint?? [??]. Intuitively, the output difference $\gamma ( \boldsymbol { x } _ { i } , j )$ of each round function should be nonzero when the input difference of the cipher is nonzero which is ensured by $\begin{array} { r } { \sum _ { i = 0 } ^ { s \cdot n - 1 } \gamma ( t x t , i ) \ge 1 } \end{array}$ . 

(2) for every $\begin{array} { r } { 1 \leq i < r , \sum _ { j = 1 } ^ { i } \sum _ { b \in \Theta _ { j } } b \geq N _ { i } } \end{array}$ is added, where $\Theta _ { j }$ denotes the set of Boolean variables modeling the activeness of the S-boxes in the ??-th round and $N _ { i }$ is a lower bound of the number of active S-boxes of ??-round differential characteristics. Intuitively, the number of active S-boxes of ?? -round differential characteristics that occurs in the first ??-rounds should be no less than $N _ { i }$ . Furthermore, $\begin{array} { r } { \sum _ { j = r - i } ^ { r } \sum _ { b \in \Theta _ { j } } b \geq N _ { i } } \end{array}$ is also added if the first ??-rounds and the $r - 1$ to ?? rounds are the same. 


Table 14. Results of the bit-wise approach without the bounding condition, Timeout is 24 hours.


<table><tr><td colspan="2">Rounds</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td></tr><tr><td rowspan="2">ASCON (12)</td><td>#AS</td><td>1</td><td>4</td><td>15</td><td colspan="12">N/A</td></tr><tr><td>Time</td><td>1s</td><td>31s</td><td>6913s</td><td colspan="12">Timeout</td></tr><tr><td rowspan="2">ELEPHANT (80)</td><td>#AS</td><td>1</td><td>2</td><td>4</td><td>6</td><td>10</td><td>12</td><td>14</td><td>16</td><td>18</td><td>20</td><td>22</td><td colspan="4">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>2s</td><td>12s</td><td>66s</td><td>428s</td><td>761s</td><td>2040s</td><td>3834s</td><td>7557s</td><td>22957s</td><td colspan="4">Timeout</td></tr><tr><td rowspan="2">GIFT-COFB (40)</td><td>#AS</td><td>1</td><td>2</td><td>3</td><td>5</td><td>7</td><td>10</td><td>13</td><td>17</td><td>19</td><td>21</td><td>23</td><td colspan="4">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>6s</td><td>26s</td><td>49s</td><td>1074s</td><td>3578s</td><td>5744s</td><td>42615s</td><td>66725s</td><td colspan="4">Timeout</td></tr><tr><td rowspan="2">GIFT (28)</td><td>#AS</td><td>1</td><td>2</td><td>3</td><td>5</td><td>7</td><td>10</td><td>13</td><td>16</td><td>18</td><td>20</td><td>22</td><td>24</td><td>26</td><td>28</td><td>30</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>2s</td><td>4s</td><td>6s</td><td>38s</td><td>240s</td><td>276s</td><td>308s</td><td>6412s</td><td>3450s</td><td>9672s</td><td>13890s</td><td>51175s</td></tr><tr><td rowspan="2">PRESENT (31)</td><td>#AS</td><td>1</td><td>2</td><td>4</td><td>6</td><td>10</td><td>12</td><td>14</td><td>16</td><td>18</td><td>20</td><td>22</td><td>24</td><td>26</td><td colspan="2">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>0s</td><td>2s</td><td>5s</td><td>11s</td><td>54s</td><td>27s</td><td>277s</td><td>745s</td><td>3515s</td><td>7750s</td><td>33215s</td><td colspan="2">Timeout</td></tr><tr><td rowspan="2">RECTANGLE (25)</td><td>#AS</td><td>1</td><td>2</td><td>3</td><td>4</td><td>6</td><td>8</td><td>11</td><td>13</td><td>15</td><td>17</td><td>19</td><td>21</td><td>23</td><td colspan="2">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>1s</td><td>3s</td><td>7s</td><td>20s</td><td>27s</td><td>115s</td><td>292s</td><td>1683s</td><td>2322s</td><td>28772s</td><td colspan="2">Timeout</td></tr><tr><td rowspan="2">SKINNY (36)</td><td>#AS</td><td>1</td><td>2</td><td>5</td><td>8</td><td>12</td><td>16</td><td>26</td><td>36</td><td>41</td><td>46</td><td>51</td><td>55</td><td colspan="3">N/A</td></tr><tr><td>Time</td><td>0s</td><td>0s</td><td>1s</td><td>2s</td><td>6s</td><td>12s</td><td>88s</td><td>291s</td><td>1672s</td><td>3337s</td><td>7391s</td><td>25231s</td><td colspan="3">Timeout</td></tr><tr><td rowspan="2">SPARKLE (7)</td><td>#AS</td><td>1</td><td>2</td><td>5</td><td>6</td><td>7</td><td colspan="10">N/A</td></tr><tr><td>Time</td><td>2s</td><td>44s</td><td>1624s</td><td>4416s</td><td>17592s</td><td colspan="10">Timeout</td></tr></table>

## E.5 Results of the Bit-wise Approach without Bounding Condition

The results of our bit-wise approach without adding the bounding condition are shown in Table 14. By comparing with the results given in Table 6, we can observe that on ASCON, ELEPHANT, PRESENT, RECTANGLE and SPARKLE, the bit-wise approach with the bounding condition achieves a better performance than the one without the bounding condition. in particular, the improvement brought by the bounding condition on RECTANGLE is more than 10 times for large round numbers. However, the bit-wise approach with bounding condition performs worse than the one without the bounding condition on GIFT-COFB and GIFT. This indicates that the bounding condition does not necessarily improve MILP solving efficiency, but overall it is recommended to be used. 

## F SUMMARY OF RELATED WORK

To facilitate the comparison with the related work, we give a summary of the related work in Table 15, which lists the existing MILP/SAT/SMT-based approaches for the security analysis of cryptographic primitives against differential cryptanalysis. 

In Table 15, the second column shows the supported operations in the respective work; the third column indicates whether its modeling is word-wise, bit-wise or extended bit-wise (Note that the extended bit-wise here means that the probabilities of bit-level input and output differences of operations are encoded in constraints); the fourth column gives the techniques used for S-box modeling (in bit-wise or extended bit-wise approach) and the last column gives the main strategy for the security analysis, recall that #AS denotes the lower bound of the minimum number of active S-boxes and Pr denotes the upper bound of the probability of optimal differential characteristics. 

In the techniques shown in the fourth column, the H-representation (H-Rep.) generation of convex hull, the logic condition (Log.) method, the conjunctive normal form (CNF) of Boolean functions, the methods in [Boura and Coggia 2020] (Alg.1, Alg.2, Alg.3 and Proposition 3), the SuperBall method and the method in [Udovenko 2021] applying monotone Boolean functions are the methods for generating linear inequalities for modeling the possible differential propagation in S-boxes; the greedy algorithm (greedy Alg.) and the MILP-based algorithm in [Sasaki and Todo 2017] (MILP Alg.) are the methods for selecting a subset of the generated inequalities. 

Received 2023-07-11; accepted 2023-11-07 


Table 15. Summary of existing MILP/SAT/SMT-based approaches, where H-Rep. denotes H-representation and Log. denotes logical condition modeling


<table><tr><td>Ref</td><td>Supported operations</td><td>Word-/(Extended) Bit-wise</td><td><eq>\Psi_S</eq> for S-box</td><td>Evaluation Strategy</td></tr><tr><td>[Mouha et al. 2011]</td><td>XOR, S-box, Linear transformation</td><td>Word-wise</td><td>N/A</td><td>MILP+#AS</td></tr><tr><td>[Sun et al. 2013]</td><td>XOR, S-box, Linear transformation</td><td>Bit-wise</td><td>None</td><td>MILP+#AS</td></tr><tr><td>[Sun et al. 2014a]</td><td>XOR, AND, S-box, Linear transformation</td><td>Bit-wise</td><td>H-Rep.+gready Alg.</td><td>MILP+#AS</td></tr><tr><td>[Sun et al. 2014b]</td><td>XOR, AND, S-box, Linear transformation</td><td>(Extended) Bit-wise</td><td>H-Rep.+gready Alg.</td><td>MILP+#AS/Pr</td></tr><tr><td>[Sasaki and Todo 2017]</td><td>XOR, S-box</td><td>Bit-wise</td><td>H-Rep./Log.+MILP Alg.</td><td>N/A</td></tr><tr><td>[Abdelkhalek et al. 2017]</td><td>XOR, S-box</td><td>Word-wise (Extended) Bit-wise</td><td>CNF+QM Alg.</td><td>MILP+#AS/Pr</td></tr><tr><td>[Boura and Coggia 2020]</td><td>XOR, S-box, Linear transformation</td><td>Bit-wise</td><td>Alg. 1, Alg. 2, Alg. 3 or Proposition 3 + MILP Alg.</td><td>N/A</td></tr><tr><td>[Sun 2021]</td><td>S-box</td><td>Bit-wise</td><td>SuperBall + MILP Alg.</td><td>N/A</td></tr><tr><td>[Li and Sun 2022]</td><td>S-box</td><td>Bit-wise</td><td>SuperBall + MILP Alg.</td><td>MILP+#AS</td></tr><tr><td>[Udovenko 2021]</td><td>S-box</td><td>Bit-wise</td><td>Monotone Boolean functions + MILP Alg.</td><td>N/A</td></tr><tr><td>[Cui et al. 2016]</td><td>XOR, S-box, modular addition</td><td>Bit-wise</td><td>[Sun et al. 2014b,a]</td><td>N/A</td></tr><tr><td>[Li et al. 2019]</td><td>XOR, S-box</td><td>Extended Bit-wise</td><td>[Sun et al. 2014b,a]</td><td>MILP+Pr</td></tr><tr><td>[Yin et al. 2017]</td><td>XOR, modular addition</td><td>Extended Bit-wise</td><td>None</td><td>MILP+Pr</td></tr><tr><td>[Ilter and Selçuk 2021]</td><td>XOR, S-box, Linear transformation</td><td>(Extended) Bit-wise</td><td>[Sun et al. 2014b] [Sasaki and Todo 2017]</td><td>MILP+#AS/Pr</td></tr><tr><td>[Zhang and Zhang 2018]</td><td>XOR, S-box, Linear transformation</td><td>Bit-wise</td><td>[Sun et al. 2014a]</td><td>MILP+#AS</td></tr><tr><td>[Fu et al. 2016]</td><td>XOR, modular addition</td><td>Extended Bit-wise</td><td>None</td><td>MILP+Pr</td></tr><tr><td>[Wang et al. 2018]</td><td>XOR, AND</td><td>Extended Bit-wise</td><td>None</td><td>MILP+Pr</td></tr><tr><td>[Zhang et al. 2018]</td><td>XOR, S-box modular addition</td><td>Extended Bit-wise</td><td>[Sun et al. 2014b,a]</td><td>MILP+Pr</td></tr><tr><td>[Zhou et al. 2019]</td><td>XOR, S-box, Linear transformation</td><td>Word-wise (Extended) Bit-wise</td><td>[Sasaki and Todo 2017] [Sun et al. 2014b,a]</td><td>MILP+#AS/Pr</td></tr><tr><td>[Mouha and Preneel 2013]</td><td>XOR, modular addition</td><td>Extended Bit-wise</td><td>None</td><td>SAT+Pr</td></tr><tr><td>[Aumasson et al. 2014]</td><td>XOR, AND</td><td>Extended Bit-wise</td><td>None</td><td>SAT/SMT+Pr</td></tr><tr><td>[Kölbl et al. 2015]</td><td>XOR, AND</td><td>Extended Bit-wise</td><td>None</td><td>SAT/SMT+Pr</td></tr><tr><td>[Song et al. 2016]</td><td>XOR, modular addition</td><td>Extended Bit-wise</td><td>None</td><td>SAT+Pr</td></tr><tr><td>[Liu et al. 2021]</td><td>XOR, S-box</td><td>(Extended) Bit-wise</td><td>None</td><td>N/A</td></tr><tr><td>[Sun et al. 2018]</td><td>XOR, S-box, Linear transformation</td><td>Extended Bit-wise</td><td>None</td><td>SAT+Pr</td></tr><tr><td>[Azimi et al. 2022]</td><td>XOR, modular addition</td><td>Extended Bit-wise</td><td>None</td><td>SMT+Pr</td></tr><tr><td>[Makarim and Rohit 2022]</td><td>XOR, S-box</td><td>(Extended) Bit-wise</td><td>[Sun et al. 2014a] [Abdelkhalek et al. 2017]</td><td>MILP/SMT+#AS/Pr</td></tr><tr><td>[Sun et al. 2021]</td><td>XOR, AND, S-box, modular addition</td><td>(Extended) Bit-wise</td><td>None</td><td>SAT+#AS/Pr</td></tr></table>