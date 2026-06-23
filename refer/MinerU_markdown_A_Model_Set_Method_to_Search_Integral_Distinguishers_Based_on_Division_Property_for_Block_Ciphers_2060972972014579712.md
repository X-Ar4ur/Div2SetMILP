# A Model Set Method to Search Integral Distinguisher Based on Division Property for Block Ciphers

Liu Zhang1,2, Huawei Liu1,2, and Zilong Wang1,2 

1 School of Cyber Engineering, Xidian University, Xi’an, China, {liuzhang@stu., liuhw@stu.,zlwang@}xidian.edu.cn 2 State Key Laboratory of Cryptology, P.O.Box 5159, Beijing 100878, China 

Abstract. In this paper, we focus on constructing an automatic search model that greatly improves efficiency with little loss of accuracy and obtains some better results in the construction of integral distinguishers for block ciphers. First, we define a new notion named BDPT Trail, which divides BDPT propagation into three parts: the division trail for K, division trailfor L, and Key-Xor operation. Secondly, we improve the insufficiency of the previous methods of calculating division trails and propose an effective algorithm that can obtain more valid division trails for L of the S-box operation. Third, we propose a new algorithm that models each Key-Xor operation based on the MILP technique for the first time. Based on this, we can accurately characterize the Key-Xor operation by solving these MILP models. After that, by selecting the appropriate initial BDPT and stopping rules, we construct an automatic search model. As a result, our automatic search model is applied to search for integral distinguishers for some block ciphers. For GIFT-64, we find a 11-round integral distinguisher, which is one more round than the previous best results. For Rectangle, we find a better 10-round integral distinguisher with 9 balanced bits, which has eight more bits than the previous best results. For Simon64, we can find more balanced bits than the previous longest distinguishers. 

Keywords: Division Property · Three-subset · MILP · Block Ciphers · Cross Propagation 

# 1 Introduction

Integral cryptanalysis is one of the most powerful cryptanalysis techniques [9]. For a set of chosen plaintexts, attackers encrypt them r rounds and calculate the value of the XOR of all ciphertexts. If the value is 0, we say that the cipher has a r-round integral distinguisher. 

Division property, a generalization of the integral property, which was proposed by Todo at EUROCRYPT 2015 [19], could explicitly describe the properties hidden between the traditional integral ALL and BALANCE properties. Later in CRYPTO 2015, Todo [18] applied the division property to MISTY1 and achieved the first theoretical integral attack of full-round MISTY1, which proves the superiority of the division property. Sun et al. [13] reviewed the division property and studied the property of a multiset satisfying certain division properties. At CRYPTO 2016, Boura and Canteaut proposed a new notion called parity set to characterize the division property of the S-box, based on which they found a better integral distinguisher for Present [4]. 

In order to exploit the algebraic structure of the round function, Todo and Morii [20] proposed the bit-based division property, which treats each bit of the target primitive independently. The bit-based division property can be divided into two categories: the conventional bit-based division property (CBDP) and the bit-based division property using three subsets (BDPT). The CBDP classify all vectors $\pmb { { u } } \in \mathbb { F } _ { 2 } ^ { n }$ into two subsets such that the parity of $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is 0 or unknown, while BDPT divides all vectors $\pmb { { u } } \in \mathbb { F } _ { 2 } ^ { n }$ into three subsets such that the parity of $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ is 0, 1 or unknown. Essentially, the set unknown in CBDP is divided into sets 1 and unknown in BDPT. Therefore, the BDPT can characterize the integral property of the primitive with more precision. For example, CBDP has found a 14-round integral distinguisher of Simon32 while BDPT has found a 15-round integral distinguisher of Simon32 [20]. However, the complexity of utilizing CBDP or BDPT is upper bounded by 2n, where n denotes the block size. 

Automatic Searching Integral Distinguishers Based on CBDP. To solve the restriction of huge complexity, Xiang et al. used the Mixed Integral Linear Programming (MILP) technique to construct an automatic search model, which was successfully applied to search integral distinguishers for lightweight ciphers whose block sizes were greater than 32 at ASIACRYPT 2016 [26]. By extending and improving the method, integral attacks have been applied to many ciphers and many better integral distinguishers have been found [14,5,21,15]. 

Automatic Searching Integral Distinguishers Based on BDPT. There are two problems in constructing an automatic search model based on BDPT. 

1. Feasibility and Efficiency. The automatic search model should be solved in practical time by openly available solvers. 

2. Accuracy and Completeness. The automatic search model needs to accurately and completely characterize the complex propagation of BDPT, which means the set K, L, and the influence of the set L on the set K should be traced. 

To address the above problems, Hu et al. [8] proposed an automatic search model for a variant three-subset division property and applied the method to improve some integral distinguishers. Later in ASIACRYPT 2019, Wang et al. [24] proposed the pruning technique that removes redundant vectors, and a new concept (“fast propagation”) that can translate BDPT into CBDP. Then, they constructed a new automatic search model based on the above techniques to search integral distinguishers. However, their methods sacrifice some accuracy of the original BDPT. The three-subset division property without unknown subset presented at EUROCRYPT 2020 [6] and the monomial prediction presented at ASIACRYPT 2020 [7] are two accurate techniques for the division property. These two techniques provide an essential description of the division property from the propagation [6] and algebraic [7] perspectives, respectively. Their main idea is to determine the parity of a public function by counting the number of solutions (or the number of monomial trails). It is the simplest solution to overcome the unknown-producing property caused by the Key-Xor operation and the cancellation property caused by the XOR operation [6]. However, when applying the technique to block ciphers directly, especially S-box-based ciphers, it is difficult to count the number of solutions in practical time by openly available solvers and the computational cost is too high. 

Our Contributions. In this paper, we construct an automatic search model that greatly improves the efficiency for block ciphers with a slight loss of accuracy and obtains some better results in the construction of integral distinguishers for block ciphers. The details of our contributions are summarized as follows. 

– BDPT Trail. We define a new notion named BDPT Trail to completely and accurately characterize the BDPT propagation. The BDPT Trail divides the BDPT propagation into three parts: the propagation of the set K, the propagation of the set L, and the Key-Xor operation. Furthermore, we introduce two notions named division trail for K and division trail for L to illustrate the propagation of sets K and L, respectively. With these notions, building an automatic search model that characterizes the BDPT propagation is equivalent to modeling the division trail for K, division trail for L, and Key-Xor operation. 

– Model the BDPT propagation of Nonlinear Layer. We first propose an “S-box” technique, which treats the nonlinear layer of a block cipher as a Blackbox, focusing only on its input and output, not on specific operations. More precisely, the “S-box” technique treats the basic operations that provide nonlinearity in non-S-box-based ciphers as an S-box. Using the “S-box” technique, we construct a generalized model that reduces the number of basic operations and models the nonlinear layer uniformly. More specifically, we transform the BDPT modeling of the nonlinear layer into the BDPT modeling of the S-box. To characterize the BDPT propagation of an S-box, we apply the method in [4,26] to calculate all the division trails for K of the S-box, and then we study the method to calculate all valid division trails for L of the S-box. We show that the method in [8] only finds a part of all valid division trails for L, and the method in [24] finds some extra invalid division trails for L. Then we present a theorem that can accurately find all valid division trails for L of the S-box according to its ANF directly. Based on this, we propose an effective algorithm that can obtain more valid division trails for L of the S-box. To model the division trails for K and division trails for L of the S-box by a set of linear inequalities whose feasible solutions are exactly these division trails, we use SageMath [17] to generate an initial set of linear inequalities and then apply a reduction algorithm to reduce the initial set such that these division trails can be modeled by the minimum number of linear inequalities [12]. 

Model the BDPT propagation of Key-Xor operation. When a Key-Xor operation is applied, new vectors generated from the set L will be added to the set K. Therefore, how to accurately characterize the Key-Xor operation is a complex problem. To solve this problem, we propose a new algorithm that models each Key-Xor operation based on the MILP technique for the first time. Based on this, we can accurately characterize the Key-Xor operation by solving these MILP models. Finally, by selecting appropriate initial BDPT and stopping rules, we can construct an automatic search model that greatly improves the efficiency for block ciphers at a little loss of accuracy, and obtain some better results. 

– Applications. We apply our automatic search model to search integral distinguishers of Simon [2], Simeck [27], Rectangle [28], Present [3] and GIFT-64 [1]. The results are shown in Table 1. 

1. For non-S-box-based block ciphers. For Simon64, we can find a better 17-round integral distinguisher with 27 balanced bits, which has four more bits than the previous longest distinguisher [24]. For Simon32, 48, 96, 128 and Simeck32, 48, 64, the distinguishers we find are in accordance with the previous longest distingui-shers [24]. 

2. For S-box-based block ciphers. For GIFT-64, we find an 11-round integral distinguisher which is one round more than the previous best results. For Rectangle, we find a 10-round integral distinguisher with 9 balanced bits, which has eight more bits than the best integral distinguisher in [11]. For Present, the distinguishers we find are in agreement with the previous longest distinguishers [24]. For the above five block ciphers, our automatic search model reduces the time complexity of searching integral distinguishers compared to the previous methods. 

Organization. This paper is organized as follows: In Section 2 we briefly review some basic background knowledge about the bit-based division property. Section 3 studies how to model these operations used in the round function of a block cipher by the MILP technique. Section 4 studies the initial and stopping rules and the search algorithm. Section 5 shows the applications of some lightweight block ciphers, and we conclude our work in Section 6. Some auxiliary materials are provided in the Appendix. 


Table 1: Summarization of integral distinguishers


<table><tr><td>Cipher</td><td>Data</td><td>Round</td><td><eq>NBB^*</eq></td><td>Time</td><td>Ref.<eq>^{\dagger}</eq></td></tr><tr><td rowspan="3">SIMON32</td><td rowspan="3"><eq>2^{31}</eq></td><td rowspan="2">15</td><td rowspan="2">3</td><td>1h48m</td><td>[23]</td></tr><tr><td>2.0m</td><td>[24]</td></tr><tr><td>15</td><td>3</td><td>1.6m</td><td>Sect. 5</td></tr><tr><td rowspan="2">SIMON48</td><td rowspan="2"><eq>2^{47}</eq></td><td>16</td><td>24</td><td>1h48m</td><td>[23]</td></tr><tr><td>16</td><td>24</td><td>8.4m</td><td>Sect. 5</td></tr><tr><td rowspan="3">SIMON64</td><td rowspan="3"><eq>2^{63}</eq></td><td rowspan="2">18</td><td rowspan="2">23</td><td>23h31m</td><td>[23]</td></tr><tr><td>1h41m</td><td>[24]</td></tr><tr><td>18</td><td>27</td><td>1h8m</td><td>Sect. 5</td></tr><tr><td rowspan="2">SIMON96</td><td rowspan="2"><eq>2^{95}</eq></td><td>22</td><td>5</td><td>31h25m</td><td>[23]</td></tr><tr><td>22</td><td>5</td><td>5h55m</td><td>Sect. 5</td></tr><tr><td rowspan="2">SIMON128</td><td rowspan="2"><eq>2^{127}</eq></td><td>26</td><td>3</td><td>62h16m</td><td>[23]</td></tr><tr><td>26</td><td>3</td><td>21h7m</td><td>Sect. 5</td></tr><tr><td rowspan="2">SIMECK32</td><td rowspan="2"><eq>2^{31}</eq></td><td>15</td><td>7</td><td>51m</td><td>[23]</td></tr><tr><td>15</td><td>7</td><td>1.3m</td><td>Sect. 5</td></tr><tr><td rowspan="2">SIMECK48</td><td rowspan="2"><eq>2^{47}</eq></td><td>18</td><td>5</td><td>5h3m</td><td>[23]</td></tr><tr><td>18</td><td>5</td><td>12.9m</td><td>Sect. 5</td></tr><tr><td rowspan="2">SIMECK64</td><td rowspan="2"><eq>2^{63}</eq></td><td>21</td><td>5</td><td>23h25m</td><td>[23]</td></tr><tr><td>21</td><td>5</td><td>47.3m</td><td>Sect. 5</td></tr><tr><td rowspan="3">PRESENT</td><td rowspan="2"><eq>2^{63}</eq></td><td rowspan="2">9</td><td rowspan="2">28</td><td>4h8m</td><td>[23]</td></tr><tr><td>10m</td><td>[24]</td></tr><tr><td><eq>2^{63}</eq></td><td>9</td><td>28</td><td>4.6m</td><td>Sect. 5</td></tr><tr><td rowspan="2">RECTANGLE</td><td><eq>2^{63}</eq></td><td>10</td><td>1</td><td>6.7m</td><td>[11]</td></tr><tr><td><eq>2^{63}</eq></td><td>10</td><td>9</td><td>5.3m</td><td>Sect. 5</td></tr><tr><td rowspan="2">GIFT-64</td><td><eq>2^{63}</eq></td><td>10</td><td>32</td><td>—</td><td>[1]</td></tr><tr><td><eq>2^{63}</eq></td><td>11</td><td>16</td><td>39.1m</td><td>Sect. 5</td></tr></table>

1. † : The paper [23] in IACR Cryptology ePrint Archive is the preprint of [24]. The results of the two papers are consistent except for the time complexity. 

2. ⋆ : The balanced bit is divided into 0 and 1, where $" 0 "$ represents the bit whose sum is $0 , \ ^ { 6 } 1 ^ { \mathfrak { n } }$ represents the bit whose sum is 1. The details results are shown in Appendix F. 

3. NBB: the number of balanced bits. 

# 2 Preliminaries

# 2.1 Notations

Let $\mathbb { F } _ { 2 }$ be the finite field {0, 1} and Fn be the n-bit string over $\mathbb { F } _ { 2 }$ . For any $a \in \mathbb { F } _ { 2 } ^ { n }$ , let a[i] be the i-th bit of a, and the Hamming weight of a is calculated as $\textstyle \sum _ { i = 0 } ^ { n - 1 } a [ i ]$ . For any $\pmb { a } = ( a _ { 0 } , \ldots , a _ { m - 1 } ) \in \mathbb { F } _ { 2 } ^ { n _ { 0 } } \times \cdots \times \mathbb { F } _ { 2 } ^ { n _ { m - 1 } }$ , the vectorial Hamming weight of a is defined as $W ( \pmb { a } ) = ( w ( a _ { 0 } ) , \dots , w ( a _ { m - 1 } ) ) \in \mathbb { Z } ^ { m }$ , where $w ( a _ { i } )$ is the Hamming weight of ${ { a } _ { i } } ,$ and Z denotes the integer ring. For any $\boldsymbol { k } \in \mathbb { Z } ^ { m }$ and $\pmb { k } ^ { \prime } \in \mathbb { Z } ^ { m }$ , we define $k \succeq k ^ { \prime } \mathrm { ~ i f ~ } k _ { i } \geqslant k _ { i } ^ { \prime }$ for all $i = 0 , 1 , \ldots , m - 1$ . Otherwise, $\boldsymbol { k } \not \subset \boldsymbol { k } ^ { \prime }$ . Let K be the set of $k ,$ , and let |K| be the number of vectors in K. Moreover, we simply write $\mathbb { K }  k$ when $\mathbb { K } : = \mathbb { K } \cup \{ k \}$ . 

Bit Product Function [19] For any $u \in \mathbb { F } _ { 2 } ^ { n }$ , let $x \in \mathbb { F } _ { 2 } ^ { n }$ be the input. The function $\pi _ { u } ( x ) : \mathbb { F } _ { 2 } ^ { n } \to \mathbb { F } _ { 2 }$ is defined as $\textstyle \pi _ { u } ( x ) : = \prod _ { i = 0 } ^ { n - 1 } x [ i ] ^ { u [ i }$ ] 

For any $\pmb { u } = ( u _ { 0 } , u _ { 1 } , \ldots , u _ { m - 1 } ) \in \mathbb { F } _ { 2 } ^ { n _ { 0 } } \times \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m - 1 } }$ , let ${ \pmb x } = ( x _ { 0 } , x _ { 1 } , \dots ,$ $x _ { m - 1 } ) \in \mathbb { F } _ { 2 } ^ { n _ { 0 } } \times \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \cdot \cdot \cdot \times \mathbb { F } _ { 2 } ^ { n _ { m - 1 } }$ 2 2 be the input. The $\pi _ { \pmb { u } } ( \pmb { x } ) : ( \mathbb { F } _ { 2 } ^ { n _ { 0 } } \times \mathbb { F } _ { 2 } ^ { n _ { 1 } } \times \dots \times$ $\mathbb { F } _ { 2 } ^ { n _ { m - 1 } } ) \to \mathbb { F } _ { 2 }$ is defined as $\begin{array} { r } { \pi _ { \pmb { u } } ( \pmb { x } ) : = \prod _ { i = 0 } ^ { m - 1 } \pi _ { u _ { i } } ( x _ { i } ) } \end{array}$ . 

# 2.2 Bit-Based Division Property

Todo and Morii proposed two types of bit-based division property (CBDP and BDPT) at FSE 2016 [20]. 

Definition 1. ((Bit-based) division property(CBDP)[20]) Let X be a multiset whose elements take a value of Fn2 . When the multiset X has the division property $\mathcal { D } _ { \mathbb { K } } ^ { 1 ^ { n } }$ , it satisfies the following conditions: 

$$
\bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {\boldsymbol {u}} (\boldsymbol {x}) = \left\{ \begin{array}{l l} u n k n o w n & \text { if   there   exists   } \boldsymbol {k} \in \mathbb {K} \text {   s.t.   } \boldsymbol {u} \succeq \boldsymbol {k}, \\ 0 & \text { otherwise }. \end{array} \right.
$$

where ${ \boldsymbol { \mathbf { \mathit { u } } } } \succeq k $ if $u _ { i } \geq k _ { i }$ for all i. 

The bit-based division property using three subsets $( \mathrm { B D P T } )$ limits the underlying space to binary domains and further expands the search scope. Namely, it introduces a new set L, which is the set of u with $\begin{array} { r } { \bigoplus _ { \pmb { x } \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } ) = 1 } \end{array}$ . 

Definition 2. (BDPT [20]) Let X be a multiset whose elements take a value $o f \mathbb { F } _ { 2 } ^ { n }$ . When the multiset X has the bit-based division property using three subsets D 1 n $\stackrel { \cdot } { \mathcal { D } } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { n ^ { - } } }$ it satisfies the following conditions: 

$$
\bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {\boldsymbol {u}} (\boldsymbol {x}) = \left\{ \begin{array}{l l} u n k n o w n & \text {if there exists $\boldsymbol{k} \in \mathbb {K} s.t. \boldsymbol{u} \succeq \boldsymbol{k}$}, \\ 1 & \text {else if there is $\boldsymbol{\ell} \in \mathbb {L} s.t. \boldsymbol{u} = \boldsymbol{\ell}$}, \\ 0 & \text {otherwise}. \end{array} \right.
$$

According to [20], if there are k, $\pmb { k } ^ { \prime } \in \mathbb { K }$ satisfying $k \succeq k ^ { \prime }$ , k can be removed from K because the vector k is redundant. Moreover, if there are $\ell \in \mathbb { L }$ and $k \in \mathbb { K }$ that satisfy $\ell \succeq k$ , the vector ℓ is redundant. For any u, the redundant vectors in K and L will not affect the parity of $\oplus _ { x \in \mathbb { X } } \pi _ { \pmb { u } } ( \pmb { x } )$ . 

Propagation Rules. We introduce only a few of the propagation rules used in the following sections. For more details, see [20]. 

Rule 1 (Key-XXor operation be $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { \mathrm { i } ^ { n } }$ 0]) L and K′ $\mathcal { D } _ { \mathbb { K } ^ { \prime } , \mathbb { L } ^ { \prime } } ^ { 1 ^ { n } }$ nput and output division property of the Key-, respectively. Assume that the round key is 

$$
\mathbb {L} ^ {\prime} \leftarrow \ell , f o r \ell \in \mathbb {L},
$$

$$
\mathbb {K} ^ {\prime} \leftarrow \boldsymbol {k}, f o r \boldsymbol {k} \in \mathbb {K},
$$

$$
\mathbb {K} ^ {\prime} \leftarrow (\ell_ {0}, \ell_ {1}, \dots , \ell_ {i} \vee 1, \dots , \ell_ {m - 1}), f o r \ell \in \mathbb {L} s a t i s f y i n g \ell_ {i} = 0.
$$

Boura et al. presented the propagation rules of S-box for K at bit-level in [4] for the first time, which is summarized in Rule 2. 

Rule 2 (S-box for K [4]) Let $F : \mathbb { F } _ { 2 } ^ { m } $ Fn be a vectorial Boolean function composed of $\left( f _ { 0 } , f _ { 1 } , \ldots , f _ { n - 1 } \right)$ , where each $f _ { i } : \mathbb { F } _ { 2 } ^ { m }  \mathbb { F } _ { 2 }$ is a Boolean function. Assuming the input to the function F is $\pmb { x } = ( x _ { 0 } , x _ { 1 } , \dots , x _ { m - 1 } ) \in \mathbb { F } _ { 2 } ^ { m }$ , the output ${ \pmb y } = ( y _ { 0 } , y _ { 1 } , \dots , y _ { n - 1 } )$ is calculated as 

$$
y _ {0} = f _ {0} \left(x _ {0}, x _ {1}, \dots , x _ {m - 1}\right),
$$

$$
y _ {1} = f _ {1} \left(x _ {0}, x _ {1}, \dots , x _ {m - 1}\right),
$$

$$
y _ {n - 1} = f _ {n - 1} \left(x _ {0}, x _ {1}, \dots , x _ {m - 1}\right).
$$

For each vector k in the input division property K, check each vector $\pmb { u } \in \mathbb { F } _ { 2 } ^ { n }$ whether the polynomial $\pi _ { u } ( y )$ contains any monomial $\pi _ { k ^ { \prime } } ( \boldsymbol { x } )$ ) satisfying $k ^ { \prime } \succeq k$ . If so, (k, u) is a valid division trail for the S-box function. 

# 2.3 MILP-Based Bit-Based Division Property

At ASIACRYPT 2016, Xiang et al. [26] applied the MILP method to the search for CBDP for the first time. With the help of the Gurobi MILP solver, they can find CBDP for block ciphers with larger block sizes, e.g. Simon128 or Present. They introduced the definition of the CBDP trail, which is defined below. 

Definition 3. (CBDP Trail [26]) Let $f _ { r }$ denote the round function of an iterated block cipher. Asinitial division property ${ \mathcal { D } } _ { \{ k \} } ^ { 1 ^ { n } } ,$ that the input multiset of the block cipher has the and denote the division property after i-round propagation through fr by $\bar { \mathcal { D } } _ { \mathbb { K } _ { i } } ^ { 1 ^ { n } }$ . Thus, we have the following chain of division property propagations: 

$$
\{\boldsymbol {k} \} \triangleq \mathbb {K} _ {0} \xrightarrow {f _ {r}} \mathbb {K} _ {1} \xrightarrow {f _ {r}} \mathbb {K} _ {2} \xrightarrow {f _ {r}} \dots
$$

Furthermore, for any vector $\pmb { k } _ { i } ^ { * } \in \mathbb { K } _ { i } ( i \geq 1 )$ , there must exist a vector $k _ { i - 1 } ^ { * } \in$ $\mathbb { K } _ { i - 1 }$ such that $k _ { i - 1 } ^ { * }$ can propagate to $\boldsymbol { k } _ { i } ^ { * }$ using the propagation rules of division properties. Furthermore, for $( \pmb { k } _ { 0 } , \pmb { k } _ { 1 } , \dots , \pmb { k } _ { r } ) \in \mathbb { K } _ { 0 } \times \mathbb { K } _ { 1 } \times \dots \times \mathbb { K } _ { r } , \ i f \ \pmb { k } _ { i - 1 }$ can propagate to $\boldsymbol { k } _ { i }$ for all $i \in \{ 1 , 2 , \ldots , r \}$ , we call $( k _ { 0 } , k _ { 1 } , \ldots , k _ { r } )$ an r-round division trail. 

Xiang et al. [26] modeled CBDP propagations of basic operations (Copy, Xor, And) and the S-box by linear inequalities. Iterating this process r times, they could build a MILP model to cover all the possible CBDP trails generated from a given initial CBDP. In our model, we will treat these basic operations as an S-box. Therefore, we only introduce the MILP models for S-box. 

Model 1 (S-box [26]) The CBDP Rule 2 in Sect. 2.2 can generate the CBDP propagation property of the S-box. Then, we use the inequality_generator function in SageMath [17] to get a set of linear inequalities. Sometimes, the number of linear inequalities in the set is large. Thus, a Greedy Algorithm [16] was proposed to reduce this set. 

# 3 Modeling BDPT Propagations

Suppose that an iterated block cipher of the round function $f _ { r }$ consists of a nonlinear layer, a linear layer, and a Key-Xor operation. Let $f _ { k }$ be the Key-Xor operation, and let $f _ { e }$ be the rest of the operations in the round function $f _ { r } .$ . To model the propagation of BDPT for the operation $f _ { e }$ and $f _ { k }$ , we define a new notion named BDPT trail. 

Definition 4. (BDPT Trail) Assume that the input multiset X to the block cipher has initial $B D P T ~ { \mathcal { D } _ { k , \ell } ^ { 1 } }$ and denote the BDPT after r-round propagation through $f _ { e }$ and $f _ { k } \ b y \ D _ { \mathbb { K } , \mathbb { L } _ { r } } ^ { 1 ^ { n } }$ , where $r \geqslant 1$ . Thus we have the following chain of BDPT propagations: 

$$
\begin{array}{l} \left\{\boldsymbol {k} \right\} \triangleq \mathbb {K} _ {0} \xrightarrow {f _ {e}} \mathbb {K} _ {1} \xrightarrow {f _ {e}} \mathbb {K} _ {2} \xrightarrow {f _ {e}} \dots \xrightarrow {f _ {e}} \mathbb {K} _ {r - 1} \xrightarrow {f _ {e}} \mathbb {K} _ {r} \\ \uparrow f _ {k} \quad \uparrow f _ {k} \quad \dots \quad \uparrow f _ {k} \quad \uparrow f _ {k} \\ \{\ell \} \triangleq \mathbb {L} _ {0} \xrightarrow {f _ {e}} \mathbb {L} _ {1} \xrightarrow {f _ {e}} \mathbb {L} _ {2} \xrightarrow {f _ {e}} \dots \xrightarrow {f _ {e}} \mathbb {L} _ {r - 1} \xrightarrow {f _ {e}} \mathbb {L} _ {r} \\ \end{array}
$$

where $\begin{array} { r } { \mathbb { K } _ { i } = f _ { e } ( \mathbb { K } _ { i - 1 } ) \cup f _ { k } ( \mathbb { L } _ { i } ) = f _ { e } ( \mathbb { K } _ { i - 1 } ) \cup f _ { k } \circ f _ { e } ( \mathbb { L } _ { i - 1 } ) , \mathbb { L } _ { i } = f _ { e } ( \mathbb { L } _ { i - 1 } ) } \end{array}$ , 1 ⩽ $i \leqslant r .$ . 

Moreover, for any vector tuple $( k _ { i } ^ { * } , \ell _ { i } ^ { * } ) , k _ { i } ^ { * } \in \mathbb { K } _ { i }$ and $\ell _ { i } ^ { * } \in \mathbb { L } _ { i } ( 1 \leqslant i \leqslant r )$ , there must exist a vector tuple $( k _ { i - 1 } ^ { * } , \ell _ { i - 1 } ^ { * } ) , k _ { i - 1 } ^ { * } \in \mathbb { K } _ { i - 1 }$ and $\ell _ { i - 1 } ^ { \ast } \ \in \ \mathbb { L } _ { i - 1 }$ , such that $( k _ { i - 1 } ^ { * } , \ell _ { i - 1 } ^ { * } )$ can propagate to $( k _ { i } ^ { * } , \ell _ { i } ^ { * } ) b y B D P T$ propagation rules. Furthermore, for $\bigl ( ( \bar { k } _ { 0 } , \ell _ { 0 } ) , ( k _ { 1 } , \ell _ { 1 } ) , \ldots , ( k _ { r } , \ell _ { r } ) \bigr ) \in \mathbb { K } _ { 0 } \times \mathbb { L } _ { 0 } \times \mathbb { K } _ { 1 } \times \mathbb { L } _ { 1 } \times \cdots \times$ $\mathbb { K } _ { r } \times \mathbb { L } _ { r } , \ i f \left( k _ { i - 1 } , \ell _ { i - 1 } \right)$ can propagate to $( k _ { i } , \ell _ { i } )$ for all $i \in \{ 1 , 2 , \ldots , r \}$ , we call $( \pmb { k } _ { 0 } , \ell _ { 0 } ) \xrightarrow { f _ { e } , f _ { k } } ( \pmb { k } _ { 1 } , \ell _ { 1 } ) \xrightarrow { f _ { e } , f _ { k } } \cdot \cdot \cdot \xrightarrow { f _ { e } , f _ { k } } ( \pmb { k } _ { r } , \pmb { \ell } _ { r } )$ an r-round BDPT trail. 

By ignoring the Key-Xor operation (which causes the vector $\ell \in \mathbb { L } _ { i } \ t o$ be added to the set $\mathbb { K } _ { i } ) _ { i }$ , we can get the following two chains, which reflect the propagation property of $f _ { e }$ . Note that the purpose of ’ignoring’ is to define two new notions to describe the BDPT propagation. 

$$
\{\boldsymbol {k} \} \triangleq \mathbb {K} _ {0} ^ {\prime} \xrightarrow {f _ {e}} \mathbb {K} _ {1} ^ {\prime} \xrightarrow {f _ {e}} \mathbb {K} _ {2} ^ {\prime} \xrightarrow {f _ {e}} \dots \xrightarrow {f _ {e}} \mathbb {K} _ {r - 1} ^ {\prime} \xrightarrow {f _ {e}} \mathbb {K} _ {r} ^ {\prime}
$$

$$
\{\ell \} \triangleq \mathbb {L} _ {0} \xrightarrow {f _ {e}} \mathbb {L} _ {1} \xrightarrow {f _ {e}} \mathbb {L} _ {2} \xrightarrow {f _ {e}} \dots \xrightarrow {f _ {e}} \mathbb {L} _ {r - 1} \xrightarrow {f _ {e}} \mathbb {L} _ {r}
$$

where $\mathbb { K } _ { i } ^ { \prime } = f _ { e } ( \mathbb { K } _ { i - 1 } ^ { \prime } ) , \mathbb { L } _ { i } = f _ { e } ( \mathbb { L } _ { i - 1 } ) , 1 \leqslant i \leqslant r .$ . 

Thus, $f o r \ ( { \pmb k } _ { 0 } ^ { \prime } , { \pmb k } _ { 1 } ^ { \prime } , \ldots , { \pmb k } _ { r } ^ { \prime } ) \in \mathbb { K } _ { 0 } ^ { \prime } \times \mathbb { K } _ { 1 } ^ { \prime } \times \dots \times \mathbb { K } _ { r } ^ { \prime } , \ i f { \pmb k } _ { i - 1 } ^ { \prime }$ can propagate to $\pmb { k } _ { i } ^ { \prime }$ for all $i \in \{ 1 , 2 , \ldots , r \}$ , we call $( k _ { 0 } ^ { \prime } , k _ { 1 } ^ { \prime } , \ldots , k _ { r } ^ { \prime } )$ an r-round division trail for K. Similarly, for $( \ell _ { 0 } , \ell _ { 1 } , \ldots , \ell _ { r } ) \in \mathbb { L } _ { 0 } \times \mathbb { L } _ { 1 } \times \cdots \times \mathbb { L } _ { r } , \ i f \ \ell _ { i - 1 }$ can propagate to $\ell _ { i }$ for all $i \in \{ 1 , 2 , \ldots , r \}$ , we call $( \ell _ { 0 } , \ell _ { 1 } , \ldots , \ell _ { r } )$ an r-round division trail for L. 

Similar to the methods in [26], for an initial $\mathrm { B D P T } ~ { \mathcal D } _ { k , \ell } ^ { 1 ^ { n } }$ , we determine whether there exist useful integral distinguishers after r-round encryption, by finding all r-round BDPT trails that start with the vector tuple $( \boldsymbol { k } , \boldsymbol { \ell } )$ . Thus, we need to accurately describe all valid division trails of the vectors k and ℓ through $f _ { e }$ and $f _ { k } .$ For the operation $f _ { e } ,$ we model the division trail for K and the division trail for $\mathbb { L } ,$ respectively. For the operation $f _ { k }$ , we construct a new MILP model to characterize the process in which part of vectors $\ell \in \mathbb { L } _ { i }$ is added to the set $\mathbb { K } _ { i }$ by the Key-Xor operation. 

# 3.1 Treat Nonlinear Layer as “S-box”

We classify block ciphers into two categories based on whether there is an S-box in the nonlinear layer. When we apply BDPT to non-S-box-based ciphers, we usually need to consider each of its specific operations for primitives. Taking the Simon family as an example, we have to consider how to represent these basic operations with a set of linear inequalities, such as Copy, Xor, And. We aim to construct a generalized model that reduces the number of basic operations and to model the non-linear layer uniformly. The intuitive idea is to regard these basic operations that provide nonlinearity as an S-box, which is named the “Sbox”. Theoretically, the core operation of the Simon family is represented by Fig. 1. We refer to the part surrounded by the red dotted line as the “S-box”. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-05-31/5cca5d38-4df8-426a-9248-3d0dc7f26d4b/9426e0e33375d5c4c8e3078521408b2cf2a0c7d576e21d56d45c2e3faf7a467c.jpg)



Fig. 1: Core operation of the Simon family [20] and “S-box”


We represent the input to the “S-box” as ${ \pmb x } = ( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } )$ , and the corresponding output as $\pmb { y } = \left( y _ { 0 } , y _ { 1 } , y _ { 2 } , y _ { 3 } \right)$ , the algebraic normal form (ANF) of the “S-box” is listed as follows: 

$$
\begin{array}{l l} y _ {0} = x _ {0}, & y _ {1} = x _ {1}, \\ y _ {2} = x _ {2}, & y _ {3} = x _ {3} \textcircled {+} x _ {3} \textcircled {+} x. \end{array} \tag {1}
$$

$$
y _ {2} = x _ {2}, \quad y _ {3} = x _ {0} x _ {1} \oplus x _ {2} \oplus x _ {3}.
$$

The S-box is an important component for most S-box-based block ciphers because it is the only nonlinear part. For non-S-box-based block ciphers, the “S-box” serves the same purpose. On the basis of this, we transform the BDPT modeling of the nonlinear layer into the BDPT modeling of the S-box. 

# 3.2 Limitation of Previously BDPT Modeling of an S-box

In [4,26], the rule was presented to calculate all the division trails for K of an S box. We study the rule to find all valid division trails for L of an S-box. 

We assume an n-bit S-box: $\mathbb { F } _ { 2 } ^ { n } \to \mathbb { F } _ { 2 } ^ { n }$ is composed of $\left( f _ { 0 } , f _ { 1 } , \ldots , f _ { n - 1 } \right)$ , where the input $\pmb { x } = ( x _ { 0 } , \ldots , x _ { n - 1 } ) \in \mathbb { F } _ { 2 } ^ { n }$ and the output $\pmb { y } = ( y _ { 0 } , \dots , y _ { n - 1 } ) \in \mathbb { F } _ { 2 } ^ { n }$ . Every yi can be expressed as a Boolean function of $( x _ { 0 } , \ldots , x _ { n - 1 } )$ , where $i \in$ $\{ 0 , \ldots , n - 1 \}$ . 

Theorem 1 ([8,24]). If the input BDPT of the S-box is $\mathcal { D } _ { k , \ell } ^ { 1 ^ { n } }$ where $\pmb { k } = ( k _ { 0 } , \ldots ,$ $k _ { n - 1 } ) , \ell = ( \ell _ { 0 } , \dots , \ell _ { n - 1 } )$ , then the output BDPT of the S-box can be calculatede $\mathcal { D } _ { \mathbb { K } , \mathbb { L } _ { 1 } } ^ { 1 ^ { n } } \ o r \ \mathcal { D } _ { \mathbb { K } , \mathbb { L } _ { 2 } } ^ { 1 ^ { n } }$ 

$$
\mathbb {K} = \left\{\boldsymbol {u} \in \mathbb {F} _ {2} ^ {n} \mid \pi_ {\boldsymbol {u}} (\boldsymbol {y}) \text {   contains   any   monomial   } \pi_ {\bar {\boldsymbol {k}}} (\boldsymbol {x}) \text {   satisfying   } \bar {\boldsymbol {k}} \succeq \boldsymbol {k} \right\}
$$

$$
\begin{array}{l} \mathbb {L} _ {1} = \{\boldsymbol {u} \in \mathbb {F} _ {2} ^ {n} \mid \pi_ {\boldsymbol {u}} (\boldsymbol {y}) d o e s n o t c o n t a i n a n y m o n o m i a l \pi_ {\bar {\ell}} (\boldsymbol {x}) s a t i s f y i n g \bar {\ell} \succ \ell \\ \quad a n d \pi_ {\boldsymbol {u}} (\boldsymbol {y}) c o n t a i n s \pi_ {\ell} (\boldsymbol {x}) \}. \end{array}
$$

$$
\mathbb {L} _ {2} = \{\boldsymbol {u} \in \mathbb {F} _ {2} ^ {n} \mid \pi_ {\boldsymbol {u}} (\boldsymbol {y}) c o n t a i n s \pi_ {\ell} (\boldsymbol {x}) \}.
$$

Remark 1. The rules K and $\mathbb { L } _ { 1 }$ are derived from [8]. Furthermore, the rules K and L2 are derived from [24]. 

There have been two previous methods to calculate all division trails for L of an S-box, as shown in Theorem 1. We briefly describe these methods as follows: firstly, by the algebraic normal form (ANF) of the S-box, each element $y _ { i }$ in the output $\pmb { y } = ( y _ { 0 } , \dots , y _ { n - 1 } ) \in \mathbb { F } _ { 2 } ^ { n }$ can be represented as a boolean function by the input $\pmb { x } = ( x _ { 0 } , \dots , x _ { n - 1 } )$ , that is, $y _ { i } = f _ { i } \left( x _ { 0 } , \dots , x _ { n - 1 } \right)$ , where $0 \leqslant i \leqslant n - 1$ . Secondly, suppose that for the input $\ell = ( \ell _ { 0 } , \dots , \ell _ { n - 1 } )$ , for every $\pmb { u } \in \mathbb { F } _ { 2 } ^ { n }$ , we calculate the πu(y) = Qn−1i=0 y[i]u[i], where y[i] = fi (x0, . . . , xn−1). Then, we $\begin{array} { r } { \pi _ { \boldsymbol { u } } ( \boldsymbol { y } ) = \prod _ { i = 0 } ^ { n - 1 } \boldsymbol { y } [ i ] ^ { \boldsymbol { u } [ i ] } } \end{array}$ $y [ i ] = f _ { i } \left( x _ { 0 } , \dots , x _ { n - 1 } \right)$ obtain $\begin{array} { r } { \pi _ { \pmb { u } } ( \pmb { y } ) = \prod _ { i = 0 } ^ { n - 1 } f _ { i } \left( x _ { 0 } , \ldots , x _ { n - 1 } \right) ^ { u [ i ] } } \end{array}$ , which is a polynomial representation about x. Finally, according to rule $\mathbb { L } _ { 1 } \mathrm { ~ o r ~ } \mathbb { L } _ { 2 }$ of Theorem 1, for the input vector $\ell ,$ check for each vector $\pmb { u } \in \mathbb { F } _ { 2 } ^ { n }$ whether the polynomial $\pi _ { u } ( y )$ contains $\pi _ { \ell } ( \pmb x )$ and $\pi _ { u } ( y )$ does not contain any monomial $\pi _ { \bar { \ell } } ( \pmb x )$ satisfying $\bar { \ell } \succ \ell ( \mathrm { o r }$ polynomial $\pi _ { u } ( y )$ contains $\pi _ { \ell } ( { \pmb x } ) )$ . If so, $( \ell , { \boldsymbol { \mathbf { \mathit { u } } } } )$ is a valid division trail for L of the S-box. 

We find that the rules L1 and $\mathbb { L } _ { 2 }$ both have some limitations, which are illustrated with two specific examples as follows. 

Example 1. (shows that the rule $\mathbb { L } _ { 1 }$ only finds a part of the division trails for L) Take the “S-box”, which represents the core operation of the Simon family as an example. The $\mathrm { ^ { 6 6 } S { - } b o x ^ { \prime } }$ is a $4 \times 4 \ \mathrm { S - b o x }$ , and its input and output are shown in Fig. 1. Assume that the input multiset X to the “S-box” has BDPT 1 4 $\mathcal { D } _ { k , \ell = ( 0 , 1 , 1 , 0 ) } ^ { 1 ^ { 4 } }$ . The process of obtaining the valid division trail $( \ell , { \boldsymbol { \mathbf { \mathit { u } } } } )$ is briefly described as follows: 


Table 2: Propagation of the bit-based division property using three subsets for the Core Operation in Simon [20]


<table><tr><td>Input <eq>\mathcal{D}_{k,\ell}^{1^4}</eq></td><td>Output <eq>\mathcal{D}_{\mathbb{K},\mathbb{L}}^{1^4}</eq></td></tr><tr><td><eq>\ell = [0,0,0,0]</eq></td><td><eq>\mathbb{L} = \{[0,0,0,0]\}</eq></td></tr><tr><td><eq>\ell = [1,0,0,0]</eq></td><td><eq>\mathbb{L} = \{[1,0,0,0]\}</eq></td></tr><tr><td><eq>\ell = [0,1,0,0]</eq></td><td><eq>\mathbb{L} = \{[0,1,0,0]\}</eq></td></tr><tr><td><eq>\ell = [1,1,0,0]</eq></td><td><eq>\mathbb{L} = \{[1,1,0,0], [0,0,0,1], [1,0,0,1], [0,1,0,1], [1,1,0,1]\}</eq></td></tr><tr><td><eq>\ell = [0,0,1,0]</eq></td><td><eq>\mathbb{L} = \{[0,0,1,0], [0,0,0,1], [0,0,1,1]\}</eq></td></tr><tr><td><eq>\ell = [1,0,1,0]</eq></td><td><eq>\mathbb{L} = \{[1,0,1,0], [1,0,0,1], [1,0,1,1]\}</eq></td></tr><tr><td><eq>\ell = [0,1,1,0]</eq></td><td><eq>\mathbb{L} = \{[0,1,1,0], [0,1,0,1], [0,1,1,1]\}</eq></td></tr><tr><td><eq>\ell = [1,1,1,0]</eq></td><td><eq>\mathbb{L} = \{[1,1,1,0], [0,0,1,1], [1,0,1,1], [0,1,1,1], [1,1,0,1]\}</eq></td></tr><tr><td><eq>\ell = [\ell_1,\ell_2,\ell_3,1]</eq></td><td><eq>\mathbb{L} = \{[\ell_1,\ell_2,\ell_3,1]\}</eq></td></tr></table>

First, we get $y _ { i } = f _ { i } \left( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } \right)$ where $0 \leqslant i \leqslant 3$ , which is a Boolean function expression about the input x, as shown in Equation (1). Second, for every $\pmb { u } \in \mathbb { F } _ { 2 } ^ { 4 } .$ , we calculate $\begin{array} { r } { \pi _ { \pmb { u } } ( \pmb { y } ) = \prod _ { i = 0 } ^ { 3 } f _ { i } \left( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } \right) ^ { u [ i ] } } \end{array}$ . For ease of understanding, we take $\pmb { u } = ( 0 , 1 , 0 , 1 ) \in \mathbb { F } _ { 2 } ^ { 4 }$ and calculate $\pi _ { ( 0 , 1 , 0 , 1 ) } ( \pmb { y } ) = y _ { 1 } y _ { 3 } =$ x1(x0x1 ⊕ x2 ⊕ x3) = x0x1 ⊕ x1x2 ⊕ x1x3. Finally, according to rule $\mathbb { L } _ { 1 }$ , we check whether $\pi _ { ( 0 , 1 , 0 , 1 ) } ( \pmb { y } ) = x _ { 0 } x _ { 1 } \oplus x _ { 1 } x _ { 2 } \oplus x _ { 1 } x _ { 3 }$ contains $\pi _ { \ell = ( 0 , 1 , 1 , 0 ) } ( \pmb { x } )$ = x1x2 and does not contain any monomial $\pi _ { \bar { \ell } } ( \pmb x )$ satisfying $\bar { \ell } \succ \ell .$ , where $\pi _ { \bar { \ell } } ( { \pmb x } ) =$ $\{ x _ { 0 } x _ { 1 } x _ { 2 } , x _ { 1 } x _ { 2 } x _ { 3 } , x _ { 0 } x _ { 1 } x _ { 2 } x _ { 3 } \} . \mathrm { ~ A ~ }$ pparently, $\pi _ { ( 0 , 1 , 0 , 1 ) } ( \pmb { y } )$ satisfies the above conditions. So $( 0 , 1 , 1 , 0 )  ( 0 , 1 , 0 , 1 )$ is a valid division trail for L of the “S-box”. To obtain all division trails for L where $\ell = ( 0 , 1 , 1 , 0 )$ , we traverse $\pmb { u } \in \mathbb { F } _ { 2 } ^ { 4 }$ and get another valid division trail for $\mathbb { L } \colon ( 0 , 1 , 1 , 0 ) \to ( 0 , 1 , 1 , 0 )$ . 

However, we find that a valid division trail for $\mathbb { L } : ( 0 , 1 , 1 , 0 ) \to ( 0 , 1 , 1 , 1 )$ appears in Table 2 by [20] which cannot be found by the rule $\mathbb { L } _ { 1 } .$ . The reason is that $\pi _ { \boldsymbol { \mathbf { u } } = ( 0 , 1 , 1 , 1 ) } ( \boldsymbol { \mathbf { \mathit { y } } } ) = x _ { 0 } x _ { 1 } x _ { 2 } \oplus x _ { 1 } x _ { 2 } \oplus \boldsymbol { \mathbf { \mathit { a } } }$ 1x2x3 contains not only $\pi _ { \ell = ( 0 , 1 , 1 , 0 ) } ( \pmb { x } ) =$ $x _ { 1 } x _ { 2 }$ but also $x _ { 0 } x _ { 1 } x _ { 2 }$ and $x _ { 1 } x _ { 2 } x _ { 3 } \in \pi _ { \bar { \ell } } ( x )$ . It is worth exploring the valid division trails for L which were missing by rule $\mathbb { L } _ { 1 }$ compared to Table 2. Therefore, for each $\boldsymbol { \ell } , \boldsymbol { u } \in \mathbb { F } _ { 2 } ^ { 4 }$ , we calculate $\pi _ { u } ( y )$ and $\pi _ { \bar { \ell } } ( \pmb x )$ where $\bar { \ell } \succ \ell ,$ and obtain all valid division trails for L of the “S-box” by rule $\mathbb { L } _ { 1 } .$ Note that the missing valid division trails for L compared to Table 2 are in bold. 

Example 2. (shows that the rule $\mathbb { L } _ { 2 }$ finds some extra invalid division trails for L) Take the Present S-box as an example. Let the input to Present S-box be ${ \pmb x } = ( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } )$ , and the corresponding output be $\pmb { y } = \left( y _ { 0 } , y _ { 1 } , y _ { 2 } , y _ { 3 } \right)$ , the algebraic normal form (ANF) of the Present S-box is shown in Equation (2). 

Assume that the input multiset X to Present S-box has BDPT $\mathcal { D } _ { k , \ell = ( 1 , 1 , 1 , 0 ) } ^ { 1 ^ { 4 } } .$ 

$$
\begin{array}{l} y _ {0} = x _ {0} x _ {1} x _ {3} \oplus x _ {0} x _ {2} x _ {3} \oplus x _ {1} x _ {2} x _ {3} \oplus x _ {1} x _ {2} \oplus x _ {0} \oplus x _ {2} \oplus x _ {3} \oplus 1 \\ y _ {1} = x _ {0} x _ {1} x _ {3} \oplus x _ {0} x _ {2} x _ {3} \oplus x _ {0} x _ {2} \oplus x _ {0} x _ {3} \oplus x _ {2} x _ {3} \oplus x _ {0} \oplus x _ {1} \oplus 1 \tag {2} \\ y _ {2} = x _ {0} x _ {1} x _ {3} \oplus x _ {0} x _ {2} x _ {3} \oplus x _ {1} x _ {2} x _ {3} \oplus x _ {0} x _ {1} \oplus x _ {0} x _ {2} \oplus x _ {0} \oplus x _ {2} \\ y _ {3} = x _ {1} x _ {2} \oplus x _ {0} \oplus x _ {1} \oplus x _ {3}. \\ \end{array}
$$

Thus, for each $\pmb { u } \in \mathbb { F } _ { 2 } ^ { 4 }$ , we calculate $\begin{array} { r } { \pi _ { \pmb { u } } ( \pmb { y } ) = \prod _ { i = 0 } ^ { 3 } f _ { i } ( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } ) ^ { u [ i ] } } \end{array}$ . For ease of understanding, we take $\pmb { u } = ( 0 , 1 , 0 , 1 ) \in \mathbb { F } _ { 2 } ^ { 4 }$ and calculate $\pi _ { ( 0 , 1 , 0 , 1 ) } ( \pmb { y } ) =$ $y _ { 1 } y _ { 3 } = ( x _ { 0 } x _ { 1 } x _ { 3 } \oplus x _ { 0 } x _ { 2 } x _ { 3 } \oplus x _ { 0 } x _ { 2 } \oplus x _ { 0 } x _ { 3 } \oplus x _ { 2 } x _ { 3 } \oplus x _ { 0 } \oplus x _ { 1 } \oplus 1 ) ( x _ { 1 } x _ { 2 } \oplus x _ { 0 } \oplus x _ { 1 } \oplus x _ { 3 } ) = 0$ $x _ { 0 } x _ { 1 } x _ { 2 } \oplus x _ { 0 } x _ { 2 } \oplus x _ { 0 } x _ { 3 } \oplus x _ { 1 } x _ { 3 } \oplus x _ { 2 } x _ { 3 } \oplus x _ { 3 }$ . According to rule L2, we check whether $\boldsymbol { \pi } _ { ( 0 , 1 , 0 , 1 ) } ( \pmb { y } ) = x _ { 0 } x _ { 1 } x _ { 2 } \oplus x _ { 0 } x _ { 2 } \oplus x _ { 0 } x _ { 3 }$ ⊕x1x3 ⊕x2x3 ⊕x3 contains $\pi _ { \ell = ( 1 , 1 , 1 , 0 ) } ( \pmb { x } ) =$ x0x1x2. Apparently, $\pi _ { ( 0 , 1 , 0 , 1 ) } ( \pmb { y } )$ satisfies this condition above, so $( 1 , 1 , 1 , 0 ) $ $( 0 , 1 , 0 , 1 )$ is a valid division trail for L of Present S-box. To obtain all the division trails for L where $\ell = ( 1 , 1 , 1 , 0 )$ , we traverse u ∈ F42 and obtain the other division trails for $\mathbb { L } \{ 1 , 1 , 1 , 0 \} \to ( 0 , 1 , 1 , 1 ) , ( 1 , 1 , 1 , 0 ) \to ( { \bar { 1 } } , 0 , 1 , 1 ) , ( 1 , 1 , 1 , 0 ) \to$ $( 1 , 1 , 0 , 1 ) , ( 1 , 1 , 1 , 0 )  ( 1 , 1 , 1 , 0 )$ and $( 1 , 1 , 1 , 0 )  ( 1 , 1 , 1 , 1 )$ . 

However, we find that the division trail $( 1 , 1 , 1 , 0 )  ( 1 , 1 , 1 , 1 )$ discovered by rule $\mathbb { L } _ { 2 }$ is an invalid division trail for L, that is, $( 1 , 1 , 1 , 0 ) \not \to ( 1 , 1 , 1 , 1 )$ . The proof is described below: 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {(1, 1, 1, 1)} (\boldsymbol {y}) \\ = \bigoplus_ {y \in \mathbb {Y}} y _ {0} y _ {1} y _ {2} y _ {3} \\ = \bigoplus_ {x \in \mathbb {X}} \left(x _ {0} x _ {1} x _ {2} x _ {3} \oplus x _ {0} x _ {1} x _ {2} \oplus x _ {0} x _ {2} x _ {3} \oplus x _ {0} x _ {2}\right) \\ = \bigoplus_ {x \in \mathbb {X}} \pi_ {(1, 1, 1, 1)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(1, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(1, 0, 1, 1)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(1, 0, 1, 0)} (\boldsymbol {x}) \\ = \text { unknown } \oplus 1 \oplus (0 \text {   or   unknown   depends   on   } \boldsymbol {k}) \oplus 0 \\ = u n k n o w n. \\ \end{array}
$$

According to Definition 2, the parity of $\begin{array} { r } { \bigoplus _ { x \in \mathbb { X } } \pi _ { ( 1 , 1 , 1 , 1 ) } ( \pmb { x } ) } \end{array}$ is unknown because ${ \pmb u } = ( 1 , 1 , 1 , 1 ) \ \succeq \ k$ for any $k ~ \in ~ \mathbb { F } _ { 2 } ^ { 4 }$ . So, the parity of $\oplus _ { y \in \mathbb { Y } } \pi _ { ( 1 , 1 , 1 , 1 ) } ( \pmb { y } )$ is unknown. In other words, $( 1 , 1 , 1 , 0 )  ( 1 , 1 , 1 , 1 )$ is an invalid division trail for L. Therefore, some extra invalid division trails for L may be obtained by the rule $\mathbb { L } _ { 2 }$ . 

From Examples 1 and 2, we show that some valid division trails for L are missing by rule $\mathbb { L } _ { 1 }$ , and some extra invalid division trails for L are produced by rule $\mathbb { L } _ { 2 } .$ . Thus, we have the following observation. 

Observation 1 The rule to calculate all the division trails for L of an S-box is between rules $\mathbb { L } _ { 1 }$ and $\mathbb { L } _ { 2 }$ . 

From Theorem 1 and Observation 1, we find that rules $\mathbb { L } _ { 1 }$ and $\mathbb { L } _ { 2 }$ only consider $\pi _ { u } ( y )$ , not the specific parity of $\textcircled { \phi } _ { u \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } )$ , which is unknown or 1 or 0. Based on this, we propose Theorem 2 and Observation 2. 

Theorem 2. Let $\ell \in \mathbb { F } _ { 2 } ^ { n }$ represent the input of an S-box. For any $\pmb { u } \in \mathbb { F } _ { 2 } ^ { n } , ( \ell , \pmb { u } )$ is a valid division trail for L if and only $i f \oplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1$ . 

Proof. Assume that the input multiset X in the S-box has BDPT $\mathcal { D } _ { k , \ell } ^ { 1 ^ { n } } ,$ where $\pmb { k } = ( k _ { 0 } , \ldots , k _ { n - 1 } ) , \pmb { \ell } = ( \ell _ { 0 } , \ldots , \ell _ { n - 1 } )$ , and the output multiset Y in the S-box has BDPT $\mathcal { D } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { n } }$ . On the one hand, for any $\pmb { \mathscr { L } } \in \mathbb { F } _ { 2 } ^ { n }$ , if (ℓ, u) is a valid division trail for $\mathbb { L } ,$ we have $\textbf { \em u } \in \mathbb { L }$ . According to Definition 2, for any $\ell ^ { \prime } \in \mathbb { L } .$ , we have $\oplus _ { y \in \mathbb { Y } } \pi \varrho \mathbf { \prime } ( \pmb { y } ) = 1$ . Thus, we get $\begin{array} { r } { \bigoplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1 } \end{array}$ . On the other hand, if $\begin{array} { r } { \bigoplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1 } \end{array}$ , we have u $\in \mathbb { L }$ by Definition 2. For any $\ell ^ { \prime } \in \mathbb { L }$ , the $( \ell , \ell ^ { \prime } )$ is a valid division trail for L. Then, we get a valid division trail for $\mathbb { L } \colon ( \ell , \boldsymbol { u } )$ . Thus, Theorem 2 is proven. 

Theorem 2 gives sufficient and necessary conditions for $( \ell , \boldsymbol { \mathbf { \mu } } _ { u } )$ to be a valid division trail for $\mathbb { L } ,$ then we propose an observation as, 

Observation 2 The parity of $\textcircled { \phi } _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } )$ is related to the vector k. 

Example 3. We take the $\mathrm { ^ { 6 6 } S ^ { \mathrm { - b o x } } } ^ { \mathrm { 7 } }$ as an example. Assume that the input multiset $\mathbb { X }$ to the “S-box” has BDPT ${ \mathcal D } _ { k , \ell = ( 0 , 1 , 1 , 0 ) } ^ { 1 ^ { 4 } }$ . In Example 1, we find a division trail for $\mathbb { L } \colon ( 0 , 1 , 1 , 0 ) \to ( 0 , 1 , 1 , 1 )$ that cannot be discovered by rule $\mathbb { L } _ { 1 }$ . Thus, 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u} = (0, 1, 1, 1)} (\boldsymbol {y}) = \bigoplus_ {y \in \mathbb {Y}} y _ {1} y _ {2} y _ {3} \\ = \bigoplus_ {x \in \mathbb {X}} \left(x _ {0} x _ {1} x _ {2} \oplus x _ {1} x _ {2} \oplus x _ {1} x _ {2} x _ {3}\right) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(1, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(0, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(0, 1, 1, 1)} (\boldsymbol {x}) \\ = (0 \text {   or   unknown   depends   on   } \boldsymbol {k}) \oplus 1 \oplus (0 \text {   or   unknown   depends   on   } \boldsymbol {k}). \\ \end{array}
$$

To illustrate that the value of k affects the parity of $\oplus _ { y \in \mathbb { Y } } \pi _ { \boldsymbol { u } = ( 0 , 1 , 1 , 1 ) } ( \boldsymbol { y } )$ , we take two specific values of the input vector k. If the input vector $\pmb { k } = ( 1 , 0 , 0 , 1 )$ , 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u} = (0, 1, 1, 1)} (\boldsymbol {y}) = \bigoplus_ {x \in \mathbb {X}} \pi_ {(1, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(0, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(0, 1, 1, 1)} (\boldsymbol {x}) \\ = 0 \oplus 1 \oplus 0 = 1. \\ \end{array}
$$

If the input vector $\pmb { k } = ( 1 , 0 , 1 , 0 )$ , 

$$
\bigoplus_ {\boldsymbol {y} \in \mathbb {Y}} \pi_ {\boldsymbol {u} = (0, 1, 1, 1)} (\boldsymbol {y}) = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(1, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(0, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(0, 1, 1, 1)} (\boldsymbol {x})
$$

# 3.3 New Modeling Method for S-box

Let the set $\mathbb { U } = \{ 0 , 1 \} ^ { n }$ represent all the elements on $\mathbb { F } _ { 2 } ^ { n }$ . For any $\textbf { \em u } \in \mathbb { F } _ { 2 } ^ { n }$ we assume that $\pi _ { u } ( y )$ contains $i \leqslant 2 ^ { n }$ monomials about the vector x, which are $\pi _ { \varphi _ { 0 } } ( \pmb { x } ) , \dots , \pi _ { \pmb { \varphi } _ { i - 1 } } ( \pmb { x } )$ , respectively. If $\pi _ { u } ( y )$ contains $\pi _ { \ell } ( \pmb x )$ , we assume that $\ell = \varphi _ { j } ,$ , where $0 \stackrel { \cdot } { \leqslant } j \leqslant i - 1$ . According to Observation 2 and Theorem 2, we propose Theorem 3, which is a more accurate method to calculate all valid division trails for L of an S-box. 

Theorem 3. If the input multiset X to the S-box has $B D P T ~ { \mathcal { D } _ { k , \ell } ^ { 1 } }$ where $k =$ $( k _ { 0 } , \ldots , k _ { n - 1 } ) , \ell = ( \ell _ { 0 } , \ldots , \ell _ { n - 1 } )$ . Let the output multiset Y of S-box have $B D P T$ $\boldsymbol { \mathcal { D } } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { n } } ,$ n 1  0 n which is calculated by 

$$
\mathbb {K} = \left\{\boldsymbol {u} \in \mathbb {F} _ {2} ^ {n} \mid \pi_ {\boldsymbol {u}} (\boldsymbol {y}) \text {   contains   any   monomial   } \pi_ {\bar {\boldsymbol {k}}} (\boldsymbol {x}) \text {   satisfying   } \bar {\boldsymbol {k}} \succeq \boldsymbol {k} \right\}
$$

$$
\mathbb {L} = \{\boldsymbol {u} \in \mathbb {F} _ {2} ^ {n} \mid \pi_ {\boldsymbol {u}} (\boldsymbol {y}) \text {   contains   } \pi_ {\boldsymbol {\ell}} (\boldsymbol {x}) \text {   and   the   input   vector   } \boldsymbol {k} \in \mathbb {S} _ {\cap} \}.
$$

where $\begin{array} { r } { \mathbb { S } _ { \cap } = \bigcap _ { q = 0 } ^ { i - 1 } \mathbb { S } _ { q } } \end{array}$ and $\mathbb { S } _ { q } = \mathbb { U } \backslash \{ \bar { \varphi } _ { q } ~ | ~ \varphi _ { q } \succeq \bar { \varphi } _ { q } \}$ . If $q \neq j$ , the $\mathbb { S } _ { q }$ represents the possible value of the input vector k when $\oplus _ { x \in \mathbb { X } } \pi _ { \varphi _ { q } } ( x ) = 0$ . If $q = j$ , the $\mathbb { S } _ { j }$ represents the possible value of the input vector k when $\textstyle \bigoplus _ { x \in \mathbb { X } } \pi _ { \varphi _ { j } } ( { \pmb x } ) = 1$ . 

The proof is provided in Appendix A. Moreover, Appendix B shows a simple example for Theorem 3. According to Theorem 3, we present a generalized algorithm to calculate all valid division trails for L of an S-box. 

We explain Algorithm 1 line by line: 

Line 1 According to input BDPT $\mathcal { D } _ { k , \ell } ^ { 1 ^ { n } }$ and Definition 2, the parity of monomial $\pi _ { \bar { k } } ( { \pmb x } )$ with $\bar { \pmb { k } } \succeq \pmb { k }$ over $\mathbb { X }$ is unknown, and we store these monomials in $F ( { \bar { X } } )$ . 

Line 2 Initialize $\bar { \mathbb { K } } \bar { \mathbb { L } }$ as empty sets and let the set $\mathbb { U } = \{ 0 , 1 \} ^ { n }$ represent all the elements on $\mathbb { F } _ { 2 } ^ { n }$ . 

Line 3-6 For any possible u, let the set $\mathbb { S } _ { \cap }$ be an empty set, and the set $\mathbb { S } _ { \cap }$ represent the intersection of the possible values of the input vector k when $\begin{array} { r } { \bigoplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1 } \end{array}$ . For i monomials contained in $\pi _ { u } ( y )$ , initialize $\mathbb { S } _ { q }$ as an empty set, $0 \leqslant q \leqslant i - 1$ . 

Line 7-8 For any possible u, if polynomial $\pi _ { u } ( y )$ contains any monomial in $F ( { \bar { X } } )$ , the parity of $\pi _ { u } ( y )$ over X is unknown. We store all these vectors u in $\bar { \mathbb K }$ . Line 9-14 For any possible u, if polynomial $\pi _ { u } ( y )$ contains the monomial $\pi _ { \ell } ( \pmb x )$ and the input vector $\begin{array} { r } { \pmb { k } \in \mathbb { S } _ { \cap } = \bigcap _ { q = 0 } ^ { i - 1 } \mathbb { S } _ { q } . } \end{array}$ where $\mathbb { S } _ { q } = \mathbb { U } \backslash \{ \bar { \varphi } _ { q } ~ | ~ \varphi _ { q } \succeq \bar { \varphi } _ { q } \}$ and $i \leqslant 2 ^ { n }$ represent $\pi _ { u } ( y )$ contains i monomials, the parity of $\pi _ { u } ( y )$ over X is 1. We store all these vectors u in $\bar { \mathbb { L } }$ . 

Line 15 $S i z e R e d u c e _ { k }$ function removes all redundant vectors in $\bar { \mathbb K }$ . Namely, if there are u, $\mathbf { { u } } ^ { \prime } \in \bar { \mathbb { K } }$ satisfying $\mathbf { \Delta } u \succeq u ^ { \prime }$ , the vector u can be removed from K¯ . Moreover, SizeReducel function removes all redundant vectors in L¯. If there are $\ell ^ { \prime } \in \bar { \mathbb { L } }$ and $\mathbf { \pmb { u } } \in \bar { \mathbb { K } }$ satisfying $\ell ^ { \prime } \succeq u$ , the vector $\ell ^ { \prime }$ can be removed from L¯. 

Line 16 Return K, L as output. 

Given an n-bit S-box and its input BDPT $\mathcal { D } _ { k , \ell } ^ { 1 ^ { n } }$ , Algorithm 1 returns the output BDPT $\boldsymbol { \mathcal { D } } _ { \mathbb { K } , \mathbb { L } } ^ { 1 ^ { n } }$ . Thus for any vector $\pmb { k } ^ { \prime } \in \mathbb { K } , ( k , \pmb { k } ^ { \prime } )$ is a valid division trail for K of the S-box. Similarly, for any vector $\ell ^ { \prime } \in \mathbb { L } , ( \ell , \ell ^ { \prime } )$ is a valid division trail for L of the S-box. Because vector ℓ does not affect the propagation of vector k through the S-box, we will obtain a complete list of the division trail for K by traversing $\boldsymbol { k } \in \mathbb { F } _ { 2 } ^ { n }$ , and Xiang et al. [26] show the results of Present S-box. Similarly, for a certain input vector $\ell \in \mathbb { F } _ { 2 } ^ { n }$ , we will obtain a set of division trails for L by traversing $\pmb { k } \in \mathbb { F } _ { 2 } ^ { n }$ . If we try all the $2 ^ { n }$ possible input vector ℓ, we will obtain a complete list of division trails for L. Table 4 in Appendix C presents a complete list of all the division trails for L of Present S-box. 


Algorithm 1: Calculating division trails of an S-box


Input: The input BDPT of an n-bit S-box $D_{k,\ell}^{1n}$ , where $k = (k_0, \ldots, k_{n-1}), \ell = (\ell_0, \ldots, \ell_{n-1})$ Output: The output BDPT $D_{K,L}^{1n}$ 1 $\bar{S} = \{\bar{k} \mid \bar{k} \succeq k\}, F(\bar{X}) = \{\pi_{\bar{k}}(\boldsymbol{x}) \mid \bar{k} \in \bar{S}\}$ 2 $\bar{K} = \emptyset, \bar{L} = \emptyset$ and $U = \{0, 1\}^n$ 3 for $u \in (\mathbb{F}_2)^n$ do<br><br>4 $S_n = \emptyset$ 5 for $0 \leqslant q \leqslant i - 1$ do<br><br>6 $| S_q = \emptyset / * i \leqslant 2^n$ represents $\pi_u(\boldsymbol{y})$ contains i monomials */<br>7 end<br><br>8 if $\pi_u(\boldsymbol{y})$ contains any monomial in $F(\bar{X})$ then<br><br>9 $| \bar{K} = \bar{K} \cup \{\boldsymbol{u}\}$ 10 end<br><br>11 if $\pi_u(\boldsymbol{y})$ contains $\pi_\ell(\boldsymbol{x})$ then<br><br>12 for $\pi_u(\boldsymbol{y})$ contains every monomial $\pi_{\varphi_q}(\boldsymbol{x})$ do<br><br>13 $| S_q = U \backslash \{\bar{\varphi}_q \mid \varphi_q \succeq \bar{\varphi}_q\}$ 14 end<br><br>15 $S_n = \bigcap_{q=0}^{i-1} S_q$ 16 if the input vector $k \in S_n$ then<br><br>17 $| \bar{L} = \bar{L} \cup \{\boldsymbol{u}\}$ 18 end<br><br>19 end<br><br>20 end<br><br>21 $K = SizeReduce_k(\bar{K})$ and $L = SizeReduce_l(\bar{L})$ 22 return K, L

Representing the Division Trails of S-box as Linear Inequalities. For an n-bit S-box, each of its valid division trail can be viewed as a 2n-dimensional vector in $\{ 0 , 1 \} ^ { 2 n }$ . Thus, all valid division trails form a subset A of $\{ 0 , 1 \} ^ { 2 n }$ . Similar to Model 1, we compute the H-Representation of the convex hull $\operatorname { C o n v } ( A )$ by using the inequality_generator function in SageMath [17]. It will return a set of linear inequalities L which characterize all valid division trails. However, L contains too many inequalities, which will make the size of the corresponding MILP problem too large to solve. Generally, the Greedy Algorithm [16] is used to reduce this set L. However, Sasaki et al. [12] found that the number of inequalities selected by the greedy algorithm was not the optimal solution, and they proposed a new reduction algorithm. We apply it to reduce this set $\mathcal { L } .$ which we showed in Algorithm 2. 

Algorithm 2: MILP-Based Select a subset of linear inequalities from $\mathcal{L}$ of an S-box

Input: A: the set of all division trails of an S-box; $\mathcal{L}$ : the set of all inequalities in the H-Represen-tation of Conv(A) with A a subset of $\{0,1\}^{2n}$ Output: $\mathcal{O}$ : a set of inequalities selected from $\mathcal{L}$ whose feasible solutions restricted in $\{0,1\}^{2n}$ are exactly A

1 $\mathcal{O} = \emptyset$ and $\mathcal{C} = \emptyset$ 2 $B = \{0,1\}^{2n} \setminus A = \{b^{(0)}, b^{(1)} \ldots, b^{(m-1)}\}$ 3 $\mathcal{L} = \{l^{(0)}, l^{(1)} \ldots, l^{(t-1)}\}$ 4 for $b^{(i)} \in B$ do

    /* $0 \leqslant i \leqslant m-1$ */
5 $\mathcal{L}^* = \emptyset$ for $l^{(j)} \in \mathcal{L}$ do
    /* $0 \leqslant j \leqslant t-1$ */
6    if the inequality $l^{(j)}$ excludes impossible division trails $b^{(i)}$ then
7    | $\mathcal{L}^* = \mathcal{L}^* \cup \{j\}$ 8    end
9    end
10 $\mathcal{C.AddConstraints}(\mathcal{L}^*)$ 11 end

12 Obj = Minimize( $\mathcal{L}$ )
13 $\mathcal{M} = ConstructModel(\mathcal{C}, Obj)$ 14 $\mathcal{O} = \mathcal{M}.Optimize()$ 15 return $\mathcal{O}$ 

We explain Algorithm 2 line by line: 

Line 1 Initialize O and C as empty sets. 

Line 2-3 Let the set $B = \{ 0 , 1 \} ^ { 2 n } \backslash A$ represent all impossible division trails, and the set L represent t inequalities obtained by using the inequality_generator function in SageMath. 

Line 4-9 For each impossible division trail $b ^ { ( i ) }$ , let the set $\mathcal { L } ^ { \ast }$ be an empty set. For any inequality $l ^ { ( j ) } \in { \mathcal { L } }$ , if the inequality $l ^ { ( j ) }$ excludes impossible division trails $b ^ { ( i ) }$ , we store the tags j of all these inequalities in $\mathcal { L } ^ { \ast }$ . AddConstraints() function adds an inequality constraint $\begin{array} { r } { \sum _ { j \in \mathcal { L } ^ { * } } z _ { j } \ \ge \ 1 } \end{array}$ to the constraint set C with the binary variables $z _ { 0 } , z _ { 1 } , \dots , z _ { t - 1 }$ , in which $z _ { j } = 1$ represents that inequality $l ^ { ( j ) }$ is chosen and $z _ { j } = 0$ represents that inequality $l ^ { ( j ) }$ will not be chosen. The constraint set C means that every impossible division trail is removed with at least one inequality. Thus, there are m constraints in the constraint set C. 

Line 10 Set the objective function $O b j \mathrm { : }$ Minimize $\sum _ { \mathrm { i } = 0 } ^ { \mathrm { t } - 1 } \mathrm { z } _ { \mathrm { j } }$ 

Line 11 ConstructModel function construct a MILP model M by using the constraint set C and the objective function $O b j$ . 

Line 12 The MILP model M is optimized by the openly available solver Gurobi. It will return a set of inequalities that is composed of all inequalities $l ^ { ( j ) } \in { \mathcal { L } }$ satisfying $z _ { j } = 1$ , where $0 \leqslant j \leqslant t - 1$ . 

Line 13 Return O as output. 

We applied Algorithm 2 to a set of linear inequalities generated with Sage-Math [17] against all valid division trails of various S-boxes. Compared to the previous reduction algorithm based on greedy algorithms, a smaller number of inequalities can be obtained by Algorithm 2. The results are shown in Table 3. To show the effectiveness of Algori-thm 2, the division trails for K and the the the division trails for L of the “S-box” is characterized by the 6 and 10 inequalities in Appendix D, respectively. 


Table 3: Number of linear inequalities to characterize all valid division trails of an S-box


<table><tr><td rowspan="2">S-box</td><td rowspan="2">The number of division trails</td><td colspan="3">#inequalities</td></tr><tr><td>SageMath</td><td>Previous</td><td>Alg. 2</td></tr><tr><td>SIMON</td><td><eq>|\mathbb{K}| = 26</eq></td><td>12</td><td>—</td><td>6</td></tr><tr><td>“S-box”</td><td><eq>|\mathbb{L}| = 30</eq></td><td>18</td><td>—</td><td>10</td></tr><tr><td>PRESENT</td><td><eq>|\mathbb{K}| = 47</eq></td><td>122</td><td>11</td><td>8</td></tr><tr><td>S-box</td><td><eq>|\mathbb{L}| = 84</eq></td><td>257</td><td>23</td><td>20</td></tr><tr><td>RECTANGLE</td><td><eq>|\mathbb{K}| = 49</eq></td><td>201</td><td>17</td><td>12</td></tr><tr><td>S-box</td><td><eq>|\mathbb{L}| = 80</eq></td><td>246</td><td>19</td><td>17</td></tr><tr><td>GIFT</td><td><eq>|\mathbb{K}| = 49</eq></td><td>218</td><td>15</td><td>12</td></tr><tr><td>S-box</td><td><eq>|\mathbb{L}| = 64</eq></td><td>236</td><td>19</td><td>16</td></tr></table>

# 3.4 BDPT Modeling of Key-Xor Operation

To model the Key-Xor operation, we propose Propositon 1 according to Definition 4. The proof is provided in Appendix E. 

Proposition 1. (Cross Propagatioiterated block cipher has initial BDPT $\begin{array} { r } { \dot { \mathcal { D } } _ { k , \ell } ^ { 1 ^ { n } } , } \end{array}$ sume th and let $\mathcal { D } _ { \mathbb { K } _ { r } , \mathbb { L } _ { 1 } } ^ { 1 ^ { n } }$ input multiset  to anr denote the BDPT of the output multiset after r-round propagation through $f _ { e }$ and $f _ { k }$ , where 

$$
\begin{array}{l} \mathbb {K} _ {r} = \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {k}) \cup \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r - 1} \circ \boldsymbol {f} _ {\boldsymbol {k}} (\mathbb {L} _ {1}) \cup \dots \cup \boldsymbol {f} _ {\boldsymbol {k}} (\mathbb {L} _ {r}) \\ = \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {k}) \cup \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r - 1} \circ \boldsymbol {f} _ {\boldsymbol {k}} \circ f _ {e} (\boldsymbol {\ell}) \cup \dots \cup \boldsymbol {f} _ {\boldsymbol {k}} \circ \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {\ell}). \\ \end{array}
$$

$$
\mathbb {L} _ {r} = \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {\ell}).
$$

Then, the set of the last vectors of all r-round division trails for K that start with the vector k, and the set of the last vectors of all r-round BDPT trails which start with the vector $\ell ,$ is equal to $\mathbb { K } _ { r }$ . Furthermore, the set of the last vectors of all r-round division trails for L, which start with the vector ℓ is equal to $\mathbb { L } _ { r }$ . 

According to Definition 4 and Propositon 1, the Key-Xor operations are independent of each other during r-round BDPT propagation. Without loss of generality, we consider the t-th Key-Xor operation, i.e., 

$$
\underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r - t} \circ \boldsymbol {f} _ {\boldsymbol {k}} \circ \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {t} (\boldsymbol {\ell}) \in \mathbb {K} _ {r}
$$

We assume that the input vector ℓ is propagated through t-round and get the set Lt. For the t-th Key-Xor operation, the input and output BDPT are $\left\{ \mathbb { L } _ { t } \right\}$ and $\{ \mathbb { K } _ { t } ^ { * } , \mathbb { L } _ { t } ^ { * } \}$ , respectively. Our model uses three n-bit variables $\mathcal { L } _ { t } , \kappa _ { t } ^ { * }$ , and $\mathcal { L } _ { t } ^ { * }$ to denote them, where n is the block size. In many block ciphers, the round key is only XORed with a part of the block. Without loss of generality, we assume that the round key is XORed with the left s $( 1 \leqslant s \leqslant n )$ bits. We consider the effect of the t-th Key-Xor operation on K∗t , L∗t , respectively. 

For the output BDPT $\mathbb { L } _ { t } ^ { * }$ , according to Rule 1, $f _ { k }$ does not affect the propagation from $\mathbb { L } _ { t }$ to $\mathbb { L } _ { t } ^ { * }$ . Therefore, the constraint on $\scriptstyle { \mathcal { L } } _ { t }$ and $\mathcal { L } _ { t } ^ { * }$ is $\mathcal { L } _ { t } ^ { * } = \mathcal { L } _ { t }$ . 

For the output BDPT $\mathbb { K } _ { t } ^ { * }$ , according to Rule 1, for every vector $\ell ^ { \ast } \in \mathbb { L } _ { t }$ satisfying $\ell _ { i } ^ { * } = 0 , 0 \leqslant i \leqslant s - 1$ , we calculate $\ell _ { i } ^ { * } \vee 1$ and add it to the set $\mathbb { K } _ { t } ^ { * }$ . Thus, the constraint on $\mathcal { L } _ { t }$ and $\boldsymbol { { \cal K } } _ { t } ^ { * }$ is $\ell _ { 0 } ^ { t } + \ell _ { 1 } ^ { t } + \cdot \cdot \cdot + \ell _ { s - 1 } ^ { t } \leqslant s - 1$ and $\mathcal { K } _ { t } ^ { * } \& \mathcal { L } _ { t } = \mathcal { L } _ { t }$ . 

We only considered the t-th Key-Xor operation $f _ { k }$ instead of considering a complete BDPT propagation chain, i.e., $\ell  \mathbb { K } _ { t } ^ { * }  \mathbb { K } _ { r }$ . We give Algorithm 3 to characterize the BDPT propagation chain based on MILP. In Algorithm 3, we construct a constraint set $\mathcal { C } _ { t }$ using a linear inequality system, which accurately characterizes the BDPT trails of the set generated by the t-th Key-Xor operation. 

We explain Algorithm 3 line by line: 

Line 1 Initialize $\mathcal { P }$ as empty sets. 

Line 2-3 In the MILP model, each n-bit variable represents the BDPT K or L. Therefore, we allocate two sets of n-bit variables ${ \boldsymbol { \kappa } } _ { i } ^ { * }$ and $\mathcal { L } _ { i }$ to represent the sets $\mathbb { K } _ { i } ^ { * }$ and $\mathbb { L } _ { i }$ , where n is the block size and $0 \leqslant i \leqslant r$ . 

Line 4 Set the objective function $O b j \mathrm { : }$ Minimize $\sum \mathrm { ^ { n - 1 } k _ { i } ^ { r } } ^ { * }$ , where $k _ { i } ^ { r ^ { * } }$ represents the i-th bit of the n-bit variables ${ \boldsymbol { \kappa } } _ { r } ^ { * }$ . 

Line 5-8 For the t-th Key-Xor operation $( 1 \leqslant t \leqslant r - 1 )$ , let the constraint set $\mathcal { C } _ { t }$ be an empty set. The former t-round BDPT propagation is characterized by linear inequalities constraint set $\mathcal { O } _ { l } ( \mathbb { L } )$ , and the inequality constraints of each round are added to the constraint set $\mathcal { C } _ { t }$ . 

Line 9-10 For the t-th Key-Xor operation, we add two new constraints $\ell _ { 0 } ^ { t } +$ $\ell _ { 1 } ^ { t } + \cdot \cdot \cdot + \ell _ { s - 1 } ^ { t } \leqslant s - 1$ and $\mathcal { K } _ { t } ^ { * } \& \mathcal { L } _ { t } = \mathcal { L } _ { t }$ to the constraint set $\mathcal { C } _ { t }$ . These two constraints can be used to obtain the vectors that can be added to the set $\boldsymbol { { \cal K } } _ { t } ^ { * }$ . 

Line 11-12 The remaining $( r - t )$ -round $\mathrm { B D P T }$ propagation is characterized by linear inequalities constraint set $\mathcal { O } _ { k } ( \mathbb { K } )$ , and the inequality constraints of each round are added to the constraint set $\mathcal { C } _ { t }$ . 


Algorithm 3: MILP-Based Characterize the Propagation Rule of Key-XOR Operations


Input: The initial input BDPT of an n-bit iterated block cipher $\mathcal{D}_{\mathbb{K}_{0}=\{\boldsymbol{k}\},\mathbb{L}_{0}=\{\boldsymbol{\ell}\}}^{1^{n}};\mathcal{O}_{k}(\mathbb{K})$ : a constraint set of linear inequalities whose feasible solutions are all valid division trails for K of the round function; $\mathcal{O}_{l}(\mathbb{L})$ : a constraint set of linear inequalities whose feasible solutions are all valid division trails for L of the round function; An n-bit vector m representing the Key-XOR operation

Output: P: A collection of MILP models with constraints for Key-XOR Operations

1 $P = \emptyset$ 2 Allocate n-bit variables $K_{i}^{*}$ to denote $K_{i}^{*}$ , where $(i = 0, 1, \ldots, r)$ 3 Allocate n-bit variables $L_{i}$ to denote $L_{i}$ , where $(i = 0, 1, \ldots, r)$ 4 Obj = Minimize( $\{k_{0}^{r^{*}} + \cdots + k_{n-1}^{r^{*}}\}$ )

5 for $(t = 1; t < r; t++)$ do

6 $C_{t} = \emptyset$ for $(i = 0; i < t; i++)$ do

7 $C_{t} \leftarrow O_{l}(L_{i}, L_{i+1})$ 8    end

9 $C_{t} \leftarrow \{\ell_{0}^{t} + \ell_{1}^{t} + \cdots + \ell_{s-1}^{t} \leqslant s - 1\}$ 10 $C_{t} \leftarrow K_{t}^{*} \& L_{t} = L_{t}$ for $(j = t; j < r; j++)$ do

11 $C_{t} \leftarrow O_{k}(K_{j}^{*}, K_{j+1}^{*})$ 12    end

13 $M_{t} = ConstructModel(C_{t}, Obj)$ 14 $P = addModel(M_{t})$ 15 end

16 return P 

Line 13 ConstructModel function constructs a MILP model $\mathcal { M } _ { t }$ using the constraint set $\mathcal { C } _ { t }$ and the objective function $O b j .$ . 

Line 14 We add the MILP model $\mathcal { M } _ { t } ,$ , which characterizes the BDPT propagation of the t-th Key-Xor operation, to the model set P. 

Line 15 Return P as output. 

Algorithm 3 constructs a MILP model for the former r−1 Key-Xor operations of an r-round block cipher, which characterizes all division trails of a complete BDPT propagation chain, i.e., $\ell \to \mathbb { K } _ { t } ^ { * } \to \mathbb { K } _ { r } , 1 \leqslant t \leqslant r - 1$ . By solving each model $\mathcal { M } _ { t }$ in the model set P separately, we can obtain the set of the last vectors of all r-round $\mathrm { B D P T }$ trails, which start with the vector ℓ. 

Remark 2. The r-th Key-Xor operation is ignored in our Algorithm 3, since it does not produce any unit vector for $\mathbb { K } _ { r }$ in our model. 

# 4 Initial, Stopping Rule and Search Algorithm

In this section, we first study the initial BDPT and stopping rule to use when searching for integral distinguishers based on BDPT. According to Definition 4 and Proposition 1, for each model which starts with the input vector k or $\ell ,$ we convert the stopping rule into an objective function of the MILP model. At last, we propose an algorithm to search integral distinguishers based on BDPT given the initial BDPT $\mathcal { D } _ { k , \ell } ^ { 1 ^ { n } }$ for an n-bit block cipher. 

# 4.1 Initial BDPT

In [20], Todo and Morii set the initial BDPT as $( k = 1 , \ell = 7 \tt f f f f f$ to search the BDPT of Simon32, where the active bits of the vector ℓ are set as 1, and the constant bit is set to 0. Similarly, we assume that $( ( k _ { 0 } ^ { 0 } , k _ { 1 } ^ { 0 } , \ldots , k _ { n - 1 } ^ { 0 } ) , ( \ell _ { 0 } ^ { 0 } , \ell _ { 1 } ^ { 0 }$ , $\dots , \ell _ { n - 1 } ^ { 0 } ) \big )$ denotes the initial BDPT, where n is the block size. The constraints on $k _ { i } ^ { 0 }$ and $\ell _ { i } ^ { 0 }$ are 

$$
k _ {i} ^ {0} = 1, \mathrm{for} i = 0, 1, 2, \ldots , n - 1
$$

$$
\ell_ {i} ^ {0} = \left\{ \begin{array}{l l} 1, & \text { if   the   } i \text {-th bit is active}, \\ 0, & \text { otherwise }. \end{array} \right.
$$

# 4.2 Stopping Rule

Stopping Rule 1 (for a single model). We consider the stopping rule of the r-th round output sets Kr and $\mathbb { L } _ { r }$ , respectively. 

According to Proposition 1, the set $\mathbb { K } _ { r }$ is composed of all r-round division trails for K, which start with the vector k, and all r-round BDPT trails produced by the Key-XOR operations, which start with the vector ℓ. In the BDPT propagation, we note that only the vector 1 can propagate to vector 1. Thus, if the given initial BDPT is $\mathcal { D } _ { k , \ell } ^ { 1 ^ { n ^ { \vee } } }$ with k = 1, the r-round division trails for K can be ignored because it does not produce any unit vector for $\mathbb { K } _ { r }$ , i.e., 

$$
\mathbb {K} _ {r} \setminus \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {k})
$$

Therefore, the model set $\mathcal { P }$ constructed by Algorithm 3 can accurately describe the vectors in $\mathbb { K } _ { r }$ . For each model $\mathcal { M } _ { t }$ in the model set P, let $( \ell _ { 0 } ^ { 0 } , \ell _ { 1 } ^ { 0 } , \dots , \ell _ { n - 1 } ^ { 0 } )$ $\xrightarrow { f _ { c } } \cdots \xrightarrow { f _ { c } } ( \ell _ { 0 } ^ { t } , \ell _ { 1 } ^ { t } , \dots , \ell _ { n - 1 } ^ { t } ) \xrightarrow { f _ { k } } ( k _ { 0 } ^ { t ^ { * } } , k _ { 1 } ^ { t ^ { * } } , \dots , k _ { n - 1 } ^ { t ^ { * } } ) \xrightarrow { f _ { c } } \cdots \xrightarrow { f _ { c } } ( k _ { 0 } ^ { r ^ { * } } , k _ { 1 } ^ { r ^ { * } } , \dots , k _ { n - 1 } ^ { r ^ { * } } )$ denote an r-round BDPT trail for the t-th Key-Xor operation. The objective function can be set as follows: 

$$
O b j: \text { Minimize } \{\mathrm{k} _ {0} ^ {\mathrm{r} ^ {*}} + \mathrm{k} _ {1} ^ {\mathrm{r} ^ {*}} + \dots + \mathrm{k} _ {\mathrm{n} - 1} ^ {\mathrm{r} ^ {*}} \}
$$

According to Proposition 1, the set $\mathbb { L } _ { r }$ is composed of all r-round division trails for $\mathbb { L } ,$ which start with the vector ℓ. Let $( \ell _ { 0 } ^ { 0 } , \ell _ { 1 } ^ { 0 } , \ldots , \ell _ { n - 1 } ^ { 0 } ) \xrightarrow { f _ { e } } \ \cdot \ \cdot \ \xrightarrow { f _ { e } }$ $( \ell _ { 0 } ^ { r } , \ell _ { 1 } ^ { r } , \dots , \ell _ { n - 1 } ^ { r } )$ denote an r-round division trail for L. Thus, we can set the objective function as : 

$$
O b j: \text { Minimize } \{\ell_ {0} ^ {\mathrm{r}} + \ell_ {1} ^ {\mathrm{r}} + \dots + \ell_ {\mathrm{n-1}} ^ {\mathrm{r}} \}
$$

Stopping Rule 2 (for the overall model). Our overall model is divided into model sets $\mathcal { P }$ and model $\mathcal { M } _ { L }$ , which describe all vectors in sets $\mathbb { K } _ { r }$ and $\mathbb { L } _ { r } ,$ respectively. Our overall MILP model only focuses on the parity of one output bit. Without loss of generality, we consider the q-th output bit. For each model $\mathcal { M } _ { t }$ in the model set ${ \mathcal { P } } _ { : }$ , we can use the solver Gurobi to determine whether the MILP model $\mathcal { M } _ { t }$ has feasible solution $K _ { q } = ( k _ { 0 } ^ { r ^ { \ast } } , \dots , k _ { n - 1 } ^ { r ^ { \ast } } )$ , where 

$$
k _ {i} ^ {r ^ {*}} = \left\{ \begin{array}{l l} 1, & \text { if } i = q, \\ 0, & \text { otherwise }. \end{array} \right.
$$

If any $\mathcal { M } _ { t }$ in the model set $\mathcal { P }$ has feasible solution $\mathinner { \mathcal { K } _ { q } } .$ , there is a unit vector $\boldsymbol { e } _ { q } \in \mathbb { K } _ { r } .$ , and further the q-th output bit is unknown. 

If there is not feasible solution $\textstyle { \mathcal { K } } _ { q }$ of model set $\mathcal { P }$ and the number of solutions $\mathcal { L } _ { q } = ( \ell _ { 0 } ^ { r ^ { * } } , \ldots , \ell _ { n - 1 } ^ { r ^ { * } } )$ of model $\mathcal { M } _ { L }$ is odd, where 

$$
\ell_ {i} ^ {r ^ {*}} = \left\{ \begin{array}{l l} 1, & \text { if } i = q, \\ 0, & \text { otherwise }. \end{array} \right.
$$

there is a unit vector $\boldsymbol { e } _ { \boldsymbol { q } } \in \mathbb { L } _ { \boldsymbol { r } }$ , and further the parity of the q-th output bit is 1. Otherwise the q-th output bit is 0. 

# 4.3 Search Algorithm

We present the automated search integral distinguishers algorithm, which decides the parity of the q-th output bit with the given initial BDPT $\mathcal { D } _ { \mathbb { K } _ { 0 } = \{ k \} , \mathbb { L } _ { 0 } = \{ \ell \} } ^ { 1 ^ { n } }$ 0 0 for an n-bit block cipher. Firstly, we allocate all round variables and auxiliary variables. Secondly, we construct a MILP model $\mathcal { M } _ { L }$ that describes all r-round division trails for L and calls Algorithm 3 to save the model set $\mathcal { P } .$ . Finally, according to the initial and stopping rules, we can obtain the parity of the q-th output bit based on BDPT. We illustrate the whole framework in Algorithm 4. 

# 5 Applications

In this section, we apply our algorithm to Simon, Simeck, Present, Rectangle and GIFT-64 block ciphers. The results are listed in Table $1 ^ { 3 }$ . In addition, for the integral distinguishers, the label $^ { 6 6 } \widehat { \mathbf { q } } ^ { , ; \rangle }$ represents the active bit, $^ { 6 6 } { \sf C } ^ { 7 3 }$ represents the constant bit, “?” represents unknown, “b” represents the balanced bit whose sum is 0 or 1, “0” represents the balanced bit whose sum is $0 , \ ^ { 6 } 1 ^ { 5 }$ represents the balanced bit whose sum is 1. 


Algorithm 4: Automated search r-round integral distinguishers


Input: The cipher E, the initial input BDPT of the n-bit block cipher $D_{K_{0}=\{k\},L_{0}=\{\ell\}}^{1^{n}}$ , and the number q

Output: The balanced information of the q-th output bit based on BDPT

1 Allocate all the variables denoting the input and output BDPT

2 $Obj = \text{Minimize}(\{\ell_{0}^{r} + \cdots + \ell_{n-1}^{r}\})$ 3 $M_{L} = ConstructModel(\{\mathcal{O}_{l}(\mathbb{L}) \times r\}, Obj)$ 4 Call Algorithm 3 and save the model set P

5 for every model $M_{t} \in P$ do

6    flag = 0

7    if the MILP model $M_{t}$ has solutions $K_{q}$ then

8    flag = flag + 1

9    end

10 end

11 if flag ≥ 1 then

12    return unknown

13 end

14 else

15    if the number of solutions $L_{q}$ of model $M_{L}$ is odd then

16    return 1

17    end

18    else

19    return 0

20    end

21 end 

# 5.1 Applications to SIMON and SIMECK

Simon [2] is a family of lightweight block ciphers published by the U.S. National Security Agency (NSA) in 2013. Simon adopts the Feistel structure, and has a very compact round function that only involves bitwise And, Xor, and Circular shift operations. The structure of one round Simon encryption is depicted in Fig. 2, where $S ^ { i }$ denotes the left circular shift by i bits. The core operation of the round function and “S-box” are represented in Fig. 1. Simeck [27] is a family of lightweight block ciphers proposed at CHES 2015, and its round function is very similar to that of Simon except for the rotation constants. We only introduce the automatic search model for Simon2n based on BDPT. 

In Algorithm 3, we introduce two constraint sets, $\mathcal { O } _ { k } ( \mathbb { K } )$ and $\mathcal { O } _ { l } ( \mathbb { L } )$ , which describe division trails for K and division trails for L of the round function, respectively. For 1-round description of Simon2n, they are similar except for the characterization of “S-box”. Therefore, we only introduce 1-round description for $\mathcal { O } _ { l } ( \mathbb { L } )$ of Simon2n. 

1-round Descriptifor L of Simon2n by $( a _ { 0 } ^ { i , 0 } , \ldots , a _ { n - 1 } ^ { i , 0 } , b _ { 0 } ^ { i , 0 } , \ldots , b _ { n - 1 } ^ { i , 0 } )  ( a _ { 0 } ^ { i + 1 , 0 } , \ldots , a _ { n - 1 } ^ { i + 1 , 0 } , b _ { 0 } ^ { i + 1 , 0 }$ , . . . , $b _ { n - 1 } ^ { i + 1 , 0 } )$ 0 n−1 0 n−1  0 n−1 0. In our model, we divide the round function of Simon2n into n ${ } ^ { 6 6 } \mathrm { S } .$ - box” operations and a Key-Xor operation. We first consider the $\mathrm { \hbar ^ { 6 } S - b o x } ^ { 3 }$ operation and assume that the input and output of the $j \mathrm { - t h } \stackrel { \scriptscriptstyle \left\{ \cdot \cdot \mathrm { C } \mathrm { - b o x } ^ { \prime \prime } \right. }  { \longrightarrow }$ are denoted as $( a _ { 0 } ^ { i , j - 1 } , \dotsc , a _ { n - 1 } ^ { i , j - 1 } , b _ { 0 } ^ { i , j - 1 } , \dotsc , b _ { n - 1 } ^ { i , j - 1 } )$ and (ai,j0 $( a _ { 0 } ^ { i , j } , \dotsc , a _ { n - 1 } ^ { i , j } , b _ { 0 } ^ { i , j } , \dotsc , b _ { n - 1 } ^ { i , j } )$ , respectively. According to ${ \mathrm { F i g . ~ } } 2 .$ , the input set that actually participates in the jth “S-box” operation is {ai,j−1(1−j) mod n, ai,j−1(8−j) mod n, ai,j−1(2−j) mod n $\{ a _ { ( 1 - j ) } ^ { i , j - 1 }$ $\ : a _ { ( 8 - j ) } ^ { i , j - 1 } \ :$ ${ } _ { n } , a _ { ( 2 - j ) } ^ { i , j - 1 }$ $\mathbf { \Phi } _ { n } ^ { - 1 } , b _ { ( n - j ) } ^ { i , j - 1 }$ , bi,j−1 (n−j) mod n}, and the corre-sponding output set is $\{ a _ { ( 1 - j ) } ^ { i , j }$ mod n $, a _ { ( 8 - j ) } ^ { i , j }$ mod ${ \bf \Phi } _ { n } , a _ { ( 2 - j ) } ^ { \imath , j }$ mod ${ \underset { - } { \neg } } b _ { ( n - j ) \atop \scriptscriptstyle \int _ { - } } ^ { i , j } \mod  \underset { - } { \} }$ bi,j(n−j) mod n}. Appendix D shows the 10 inequalities for the division trails for L of the $\mathrm { \ddot { \ s } - } \mathrm { \ddot { b } - } \mathrm { \vec { \mathrm { x } } \mathrm { \ ' } }$ , and thus the 4-bit input and output can be modeled by the 10 inequalities, which are denoted by $\mathcal { L } _ { 1 }$ . For the rest $( 2 n - 4 )$ bits, which remains unchanged, we have 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-05-31/5cca5d38-4df8-426a-9248-3d0dc7f26d4b/6100e90e5a082d40dd947df21841bab5a6ddfa5a6a38535a5c69a8b54673dff7.jpg)



Fig. 2: Round function of Simon


$$
\mathcal {L} _ {2}: \left\{ \begin{array}{l l} a _ {m} ^ {i, j} & = a _ {m} ^ {i, j - 1} \quad m \in \{0, 1, \ldots , n - 1 \} \setminus \{(1 - j), (8 - j), (2 - j) \} \bmod n \\ b _ {m} ^ {i, j} & = b _ {m} ^ {i, j - 1} \quad m \in \{0, 1, \ldots , n - 1 \} \setminus \{(n - j) \} \bmod n \end{array} \right.
$$

Therefore, we get an accurate description $\{ \mathcal { L } _ { 1 } , \mathcal { L } _ { 2 } \}$ of the division trails for L of the $j \mathrm { - t h \ ^ { \mathrm { \sc ~ t e } } { S } \mathrm { - b o x } ^ { \mathrm { \prime } } }$ . By repeating this procedure n times, we can get a set of linear inequalities for the n “S-box” operations. 

At last, we consider the Key-Xor operation, and its input and output are denoted as $( a _ { 0 } ^ { i , n } , \ldots , a _ { n - 1 } ^ { i , n } , b _ { 0 } ^ { i , n } , \ldots , b _ { n - 1 } ^ { i , n } )$ and $( a _ { 0 } ^ { i + 1 , 0 } , \ldots , a _ { n - 1 } ^ { i + 1 , 0 } , b _ { 0 } ^ { i + 1 , 0 } , \ldots , b _ { n - 1 } ^ { i + 1 , 0 } )$ respectively. According to Rule 1, the Key-Xor operation does not affect the propagation of division trails for L. Therefore, the Key-Xor operation in Simon2n can be modeled by the following inequalities: 

$$
\mathcal {L} _ {3}: \left\{ \begin{array}{l l} a _ {m} ^ {i + 1, 0} = b _ {m} ^ {i, n} & m \in \{0, 1, \ldots , n - 1 \} \\ b _ {m} ^ {i + 1, 0} = a _ {m} ^ {i, n} & m \in \{0, 1, \ldots , n - 1 \} \end{array} \right.
$$

So far, we have modeled all operations used in the round function of Simon2n and get an accurate description $\{ \{ { \mathcal { L } } _ { 1 } , { \mathcal { L } } _ { 2 } \} \times n , { \mathcal { L } } _ { 3 } \}$ of 1-round division trails for $\mathbb { L } , \ \mathrm { i . e . , } \ \mathcal { O } _ { l } ( \mathbb { L } )$ . Similarly, we can get the 1-round description for $\mathcal { O } _ { l } ( \mathbb { K } )$ of Simon2n. 

Integral Distinguishers. We use Algorithm 3 and 4 to search the integral distinguishers of the Simon and Simeck family based on BDPT. 

1. For Simon64, we can find a 17-round integral distinguisher with 27 balanced bits, which has four more bits than the previous longest distinguisher [24]. 

For Simon32, 48, 96, 128, the distinguishers we find are in accordance with the previous longest distinguishers found in [24]. 

2. For Simeck32, 48, 64, the distinguishers we find are in accordance with the previous longest distinguishers found in [26,24]. 

The detailed integral distinguishers of Simon and Simeck are listed in Appendix F. To further prove the accuracy of our automatic search model, we apply Algorithm 3 and 4 to search integral distinguishers of Simon(102) [10] based on BDPT, and show the detailed results in Appendix F.12. 

# 5.2 Applications to PRESENT, RECTANGLE and GIFT-64

Present [3] has an SPN structure and uses 80- and 128-bit keys with 64-bit blocks through 31 rounds, of which the linear layers are bit permutations. Fig. 3 illustrates the one-round structure of Present. Rectangle [28] is a bit-slice lightweight block cipher proposed in 2015, and its structure is very similar to Present. By revisiting the design strategy of Present, Banik et al. propose a new lightweight block cipher GIFT [1], which corrects the well-known weakness of Present with regards to linear hulls. 

![image](https://cdn-mineru.openxlab.org.cn/result/2026-05-31/5cca5d38-4df8-426a-9248-3d0dc7f26d4b/5ea15e299c15f06c578e15d921b4fee04f33c3e5163e2c32248b568d8d152ae1.jpg)



S  S  S  S  S  S  S  S  S  S  S  S  S  S  S Fig. 3: One-round SPN Structure of Present


Integral Distinguishers. We have shown how to model the Ol(K) and Ol(L) in Simon. Thus, we omit the details for these three ciphers due to the limit of space. We apply Algorithm 3 and 4 to search for integral distinguishers of Present, Rectangle and GIFT-64 based on BDPT. 

1. For Present, the distinguishers we find are in accordance with the previous longest distinguish-ers[24]. 

2. For Rectangle, we find a 10-round integral distinguisher with 9 balanced bits, which has eight more bits than the previous best integral distinguisher in [11]. 

3. For GIFT-64, we find a 11-round integral distinguisher, which is one more round than the previous best results [1]. 

The detailed integral distinguishers of Present, Rectangle and GIFT-64 are listed in Appendix F. 

# 6 Conclusions

In this paper, we proposed an automatic search model to search integral distinguishers for block ciphers based on BDPT. We first proposed an effective algorithm that can more accurately obtain the division trails for K and division trails for L of the S-box according to its ANF directly. Then we model each Key-Xor operation based on the MILP technique for the first time. By solving these MILP models, we could accurately characterize the Key-Xor operation. Finally, by selecting appropriate initial and stopping rules, we can construct an automatic search model that greatly improves the efficiency for block ciphers with little loss of accuracy, based on which we present an algorithm to estimate whether the q-th output bit is balanced. 

We apply our automatic search model to search for integral distinguishers of some block ciphers. For Simon64, Rectangle and GIFT-64, we obtained much better integral distinguishers than previous best results in the open literature. For other block ciphers, our results are in accordance with the previous longest distinguishers. Moreover, compared to the previous methods, our automatic search model reduces the time complexity of searching integral distinguishers for the block ciphers. 

# References



1. Banik, S., Pandey, S.K., Peyrin, T., Sasaki, Y., Sim, S.M., Todo, Y.: GIFT: A small present - towards reaching the limit of lightweight encryption. In: CHES. Lecture Notes in Computer Science, vol. 10529, pp. 321–345. Springer (2017) 





2. Beaulieu, R., Shors, D., Smith, J., Treatman-Clark, S., Weeks, B., Wingers, L.: The SIMON and SPECK lightweight block ciphers. In: DAC. pp. 175:1–175:6 (2015) 





3. Bogdanov, A., Knudsen, L.R., Leander, G., Paar, C., Poschmann, A., Robshaw, M.J.B., Seurin, Y., Vikkelsoe, C.: PRESENT: an ultra-lightweight block cipher. In: CHES. Lecture Notes in Computer Science, vol. 4727, pp. 450–466 (2007) 





4. Boura, C., Canteaut, A.: Another view of the division property. In: Robshaw, M., Katz, J. (eds.) Advances in Cryptology - CRYPTO 2016 - 36th Annual International Cryptology Conference, Santa Barbara, CA, USA, August 14-18, 2016, Proceedings, Part I. Lecture Notes in Computer Science, vol. 9814, pp. 654–682 (2016) 





5. Funabiki, Y., Todo, Y., Isobe, T., Morii, M.: Improved integral attack on HIGHT. In: ACISP (1). Lecture Notes in Computer Science, vol. 10342, pp. 363–383. Springer (2017) 





6. Hao, Y., Leander, G., Meier, W., Todo, Y., Wang, Q.: Modeling for three-subset division property without unknown subset - improved cube attacks against trivium and grain-128aead. In: EUROCRYPT (1). Lecture Notes in Computer Science, vol. 12105, pp. 466–495. Springer (2020) 





7. Hu, K., Sun, S., Wang, M., Wang, Q.: An algebraic formulation of the division property: Revisiting degree evaluations, cube attacks, and key-independent sums. In: ASIACRYPT (1). Lecture Notes in Computer Science, vol. 12491, pp. 446–476. Springer (2020) 





8. Hu, K., Wang, M.: Automatic search for a variant of division property using three subsets. In: CT-RSA. Lecture Notes in Computer Science, vol. 11405, pp. 412–432 (2019) 





9. Knudsen, L.R., Wagner, D.A.: Integral cryptanalysis. In: Daemen, J., Rijmen, V. (eds.) Fast Software Encryption, 9th International Workshop, FSE 2002, Leuven, Belgium, February 4-6, 2002, Revised Papers. Lecture Notes in Computer Science, vol. 2365, pp. 112–127 (2002) 





10. Kölbl, S., Leander, G., Tiessen, T.: Observations on the SIMON block cipher family. In: CRYPTO (1). Lecture Notes in Computer Science, vol. 9215, pp. 161–185. Springer (2015) 





11. Lambin, B., Derbez, P., Fouque, P.: Linearly equivalent s-boxes and the division property. Des. Codes Cryptogr. 88(10), 2207–2231 (2020) 





12. Sasaki, Y., Todo, Y.: New algorithm for modeling s-box in MILP based differential and division trail search. In: SECITC. Lecture Notes in Computer Science, vol. 10543, pp. 150–165 (2017) 





13. Sun, B., Hai, X., Zhang, W., Cheng, L., Yang, Z.: New observation on division property. Sci. China Inf. Sci. 60(9), 98102 (2017) 





14. Sun, L., Wang, W., Wang, M.: Automatic search of bit-based division property for ARX ciphers and word-based division property. In: Takagi, T., Peyrin, T. (eds.) Advances in Cryptology - ASIACRYPT 2017 - 23rd International Conference on the Theory and Applications of Cryptology and Information Security, Hong Kong, China, December 3-7, 2017, Proceedings, Part I. Lecture Notes in Computer Science, vol. 10624, pp. 128–157 (2017) 





15. Sun, L., Wang, W., Wang, M.: Milp-aided bit-based division property for primitives with non-bit-permutation linear layers. IET Inf. Secur. 14(1), 12–20 (2020) 





16. Sun, S., Hu, L., Wang, P., Qiao, K., Ma, X., Song, L.: Automatic security evaluation and (related-key) differential characteristic search: Application to simon, present, lblock, DES(L) and other bit-oriented block ciphers. In: Sarkar, P., Iwata, T. (eds.) Advances in Cryptology - ASIACRYPT 2014 - 20th International Conference on the Theory and Application of Cryptology and Information Security, Kaoshiung, Taiwan, R.O.C., December 7-11, 2014. Proceedings, Part I. Lecture Notes in Computer Science, vol. 8873, pp. 158–178 (2014) 





17. The Sage Developers: SageMath, the Sage Mathematics Software System (Version 9.1.0) (2020), https://www.sagemath.org 





18. Todo, Y.: Integral cryptanalysis on full MISTY1. In: Gennaro, R., Robshaw, M. (eds.) Advances in Cryptology - CRYPTO 2015 - 35th Annual Cryptology Conference, Santa Barbara, CA, USA, August 16-20, 2015, Proceedings, Part I. Lecture Notes in Computer Science, vol. 9215, pp. 413–432 (2015) 





19. Todo, Y.: Structural evaluation by generalized integral property. In: Oswald, E., Fischlin, M. (eds.) Advances in Cryptology - EUROCRYPT 2015 - 34th Annual International Conference on the Theory and Applications of Cryptographic Techniques, Sofia, Bulgaria, April 26-30, 2015, Proceedings, Part I. Lecture Notes in Computer Science, vol. 9056, pp. 287–314 (2015) 





20. Todo, Y., Morii, M.: Bit-based division property and application to simon family. In: Peyrin, T. (ed.) Fast Software Encryption - 23rd International Conference, FSE 2016, Bochum, Germany, March 20-23, 2016, Revised Selected Papers. Lecture Notes in Computer Science, vol. 9783, pp. 357–377 (2016) 





21. Wang, Q., Grassi, L., Rechberger, C.: Zero-sum partitions of PHOTON permutations. In: CT-RSA. Lecture Notes in Computer Science, vol. 10808, pp. 279–299 (2018) 





22. Wang, Q., Liu, Z., Varici, K., Sasaki, Y., Rijmen, V., Todo, Y.: Cryptanalysis of reduced-round SIMON32 and SIMON48. In: INDOCRYPT. Lecture Notes in Computer Science, vol. 8885, pp. 143–160 (2014) 





23. Wang, S., Hu, B., Guan, J., Zhang, K., Shi, T.: MILP method of searching integral distinguishers based on division property using three subsets. IACR Cryptol. ePrint Arch. p. 1186 (2018) 





24. Wang, S., Hu, B., Guan, J., Zhang, K., Shi, T.: Milp-aided method of searching division property using three subsets and applications. In: ASIACRYPT (3). Lecture Notes in Computer Science, vol. 11923, pp. 398–427 (2019) 





25. Wang, S., Hu, B., Guan, J., Zhang, K., Shi, T.: Exploring secret keys in searching integral distinguishers based on division property. IACR Trans. Symmetric Cryptol. 2020(3), 288–304 (2020) 





26. Xiang, Z., Zhang, W., Bao, Z., Lin, D.: Applying MILP method to searching integral distinguishers based on division property for 6 lightweight block ciphers. In: Cheon, J.H., Takagi, T. (eds.) Advances in Cryptology - ASIACRYPT 2016 - 22nd International Conference on the Theory and Application of Cryptology and Information Security, Hanoi, Vietnam, December 4-8, 2016, Proceedings, Part I. Lecture Notes in Computer Science, vol. 10031, pp. 648–678 (2016) 





27. Yang, G., Zhu, B., Suder, V., Aagaard, M.D., Gong, G.: The simeck family of lightweight block ciphers. In: CHES. Lecture Notes in Computer Science, vol. 9293, pp. 307–329 (2015) 





28. Zhang, W., Bao, Z., Lin, D., Rijmen, V., Yang, B., Verbauwhede, I.: RECTANGLE: a bit-slice lightweight block cipher suitable for multiple platforms. Sci. China Inf. Sci. 58(12), 1–15 (2015) 



# A Proof of Theorem 3

According to Theorem 2, for the input vector ℓ, calculating $\textcircled { \phi } _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } )$ for all $\pmb { u } \in \mathbb { F } _ { 2 } ^ { n } , \mathrm { i f } \oplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1$ , we obtain a valid division trail for $\mathbb { L } { : } ( \ell , { \boldsymbol { u } } )$ . In order to obtain the condition that $\begin{array} { r } { \bigoplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1 } \end{array}$ is established, we first study the $\pi _ { u } ( y )$ , which is a polynomial representation about x. 

$$
\begin{array}{l} \pi_ {\boldsymbol {u}} (\boldsymbol {y}) = \prod_ {i = 0} ^ {n - 1} y [ i ] ^ {u [ i ]}, \text {   where   } y [ i ] = f _ {i} (x _ {0}, \dots , x _ {n - 1}) \\ = \prod_ {i = 0} ^ {n - 1} f _ {i} \left(x _ {0}, \dots , x _ {n - 1}\right) ^ {u [ i ]} \\ = \prod_ {i = 0} ^ {n - 1} \left(\bigoplus_ {\boldsymbol {v} \in \mathbb {F} _ {2} ^ {n}} a _ {\boldsymbol {v}} ^ {f _ {i}} \pi_ {\boldsymbol {v}} (\boldsymbol {x})\right) ^ {u [ i ]} \\ \end{array}
$$

Let 

$$
g (\boldsymbol {x}) = \prod_ {i = 0} ^ {n - 1} \left(\bigoplus_ {\boldsymbol {v} \in \mathbb {F} _ {2} ^ {n}} a _ {\boldsymbol {v}} ^ {f _ {i}} \pi_ {\boldsymbol {v}} (\boldsymbol {x})\right) ^ {u [ i ]} = \bigoplus_ {\varphi \in \mathbb {F} _ {2} ^ {n}} a _ {\varphi} ^ {g} \pi_ {\varphi} (\boldsymbol {x})
$$

where $a _ { v } ^ { f _ { i } } \in \mathbb { F } _ { 2 }$ is a constant value depending on $f _ { i }$ and v, $a _ { \varphi } ^ { g } \in \mathbb { F } _ { 2 }$ also is a constant value depending on g and $\varphi$ . Thus, 

$$
\pi_ {\boldsymbol {u}} (\boldsymbol {y}) = \bigoplus_ {\varphi \in \mathbb {F} _ {2} ^ {n}} a _ {\varphi} ^ {g} \pi_ {\varphi} (\boldsymbol {x})
$$

we assume the set $\mathbb { O } = \{ \varphi \in \mathbb { F } _ { 2 } ^ { n } \mid a _ { \varphi } ^ { g } = 1 \}$ , we have 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u}} (\boldsymbol {y}) = \bigoplus_ {x \in \mathbb {X}} \bigoplus_ {\varphi \in \mathbb {F} _ {2} ^ {n}} a _ {\varphi} ^ {g} \pi_ {\varphi} (\boldsymbol {x}) \\ = \bigoplus_ {x \in \mathbb {X}} \pi_ {\varphi_ {0}} (\boldsymbol {x}) \oplus \dots \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {\varphi_ {| 0 | - 1}} (\boldsymbol {x}) \\ \end{array}
$$

where the $\varphi _ { i } \in \mathbb { O } , 0 \leq i \leq | \mathbb { O } | - 1$ . According to Definition 2, the parity of $\oplus _ { x \in \mathbb { X } } \pi _ { \varphi _ { i } } ( { \pmb x } )$ is unknown or 1 or 0, where $0 \leq i \leq | \mathbb { O } | - 1 . \ \mathrm { I f } \oplus _ { y \in \mathbb { Y } } \pi _ { \pmb { u } } ( \pmb { y } ) = 1$ , i.e., 

$$
\bigoplus_ {x \in \mathbb {X}} \pi_ {\varphi_ {0}} (\boldsymbol {x}) \oplus \dots \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {\varphi_ {| 0 | - 1}} (\boldsymbol {x}) = 1
$$

Obviously, the input vector $\ell \in \mathbb { O }$ and $\begin{array} { r } { \bigoplus _ { x \in \mathbb { X } } \pi \varrho ( \pmb { x } ) = 1 } \end{array}$ . Let $\ell = \varphi _ { j }$ , where $j \in \{ i \mid 0 \leq i \leq | \mathbb { O } | - 1 \}$ . For any $\varphi _ { i } \in \mathbb { O } \backslash \{ \varphi _ { j } \} , \bigoplus _ { x \in \mathbb { X } } \pi _ { \varphi _ { i } } ( x ) = 0$ . Thus, 

$$
\bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u}} (\boldsymbol {y}) = 1 \Leftrightarrow \text {   for   every   } \varphi_ {i} \in \mathbb {O} \backslash \{\varphi_ {j} \},
$$

$$
\bigoplus_ {x \in \mathbb {X}} \pi_ {\varphi_ {i}} (\boldsymbol {x}) = 0 \text { and } \bigoplus_ {x \in \mathbb {X}} \pi_ {\ell = \varphi_ {j}} (\boldsymbol {x}) = 1
$$

According to Definition 2, for every $\begin{array} { r } { \bigoplus _ { { \boldsymbol { x } } \in \mathbb { X } } \pi _ { \boldsymbol { \varphi } _ { i } } ( { \boldsymbol { \pmb { x } } } ) = 0 } \end{array}$ and $\begin{array} { r } { \bigoplus _ { x \in \mathbb { X } } \pi _ { \varphi _ { i } } ( \pmb { x } ) = 1 } \end{array}$ , the possible value of the input vector k is $\dot { \mathbb { S } } _ { i } = \mathbb { U } \backslash \{ \bar { \varphi } _ { i } | \varphi _ { i } \succeq \bar { \varphi _ { i } } \}$ , where $0 \leq i \leq | \mathbb { O } | - 1$ (Because $j \in \{ i \mid 0 \leq i \leq | \mathbb { O } | - 1 \}$ , we have $0 \leq i \leq | \mathbb { O } | - 1 )$ . Therefore, for all $0 \leq i \leq | \mathbb { O } | - 1$ , we have 

$$
\mathbb {S} _ {\cap} = \mathbb {S} _ {0} \cap \dots \cap \mathbb {S} _ {| \mathbb {O} | - 1}
$$

The set $\mathbb { S } _ { \cap }$ represents the intersection of the possible values of k when the $\begin{array} { r } { \bigoplus _ { \substack { x \in \mathbb { X } ^ { \pi } \varphi _ { i } } } ( \pmb { x } ) = 0 } \end{array}$ and $\begin{array} { r } { \bigoplus _ { \substack { x \in \mathbb { X } ^ { \pi } \varphi _ { i } } } ( \pmb { x } ) = 1 } \end{array}$ . If the input vector $\boldsymbol { k } \in \mathbb { S } _ { \cap } .$ , it means that the input vector k such that (A) holds. So (A) is equivalent to $\pi _ { u } ( y )$ contains $\pi _ { \ell } ( \pmb x )$ and $\pmb { k } \in \mathbb { S } _ { \cap }$ . 

# B Example for Theorem 3

To help readers understand the Theorem 3, we show a simple example of the core operation of Simon family. We first treat the core operation of Simon family as an “S-box”, which isFig. 1. Let the input m ${ \mathrm { ~ a ~ 4 ~ } } \times { \mathrm { ~ 4 ~ } } { \mathrm { S } } .$ -box, to the $\mathrm { ^ { 6 6 } S { - } b o x ^ { \prime } }$ nput and ou have BDPT ${ \mathcal D } _ { k , \ell = ( 0 , 0 , 1 , 0 ) } ^ { 1 ^ { 4 } } .$ n in For every $\pmb { u } \in \mathbb { F } _ { 2 } ^ { 4 }$ , we calculate the $\begin{array} { r } { \pi _ { \pmb { u } } ( \pmb { y } ) = \prod _ { i = 0 } ^ { 3 } f _ { i } \left( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } \right) ^ { u \left[ i \right] } } \end{array}$ as shown in Table 4. According to Theorem 3 and Table 4, if the $\pi _ { u } ( y )$ contains $\pi _ { \ell } ( \pmb x )$ , the vector u $\in \ \{ ( 0 , 0 , 0 , 1 ) , ( 0 , 0 , 1 , 0 ) , ( 0 , 0 , 1 , 1 ) \}$ }. We consider the following three cases respectively. 

1. When the vector $\pmb { u } = ( 0 , 0 , 0 , 1 )$ , we calculate: 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u} = (0, 0, 0, 1)} (\boldsymbol {y}) \\ = \bigoplus_ {y \in \mathbb {Y}} y _ {3} \\ = \bigoplus_ {x \in \mathbb {X}} \left(x _ {0} x _ {1} \oplus x _ {2} \oplus x _ {3}\right) \\ = \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(1, 1, 0, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(0, 0, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {\boldsymbol {x} \in \mathbb {X}} \pi_ {(0, 0, 0, 1)} (\boldsymbol {x}) \\ = (0 \text {   or   unknown   Depends   on   } \boldsymbol {k}) \oplus 1 \oplus (0 \text {   or   unknown   Depends   on   } \boldsymbol {k}) \\ \end{array}
$$

if $\bigoplus _ { x \in \mathbb { X } } \pi _ { ( 1 , 1 , 0 , 0 ) } ( x ) = 0$ , the vector ${ \pmb k } \in \mathbb { S } _ { 0 } = \mathbb { U } \backslash \{ { \bar { \varphi } } _ { 0 } ~ | ~ \varphi _ { 0 } = ( 1 , 1 , 0 , 0 ) ~ \succeq$ $\langle { \bar { \varphi } } _ { 0 } \rangle$ . Similarly, we can get $\pmb { k } \in \mathbb { S } _ { 1 } = \mathbb { U } \backslash \{ \bar { \varphi } _ { 1 } \ | \ \varphi _ { 1 } = ( 0 , 0 , 1 , 0 ) \ \succeq \bar { \varphi } _ { 1 } \}$ and $\pmb { k } \in \mathbb { S } _ { 2 } = \mathbb { U } \backslash \{ \bar { \varphi } _ { 2 } \mid \varphi _ { 2 } = ( 0 , 0 , 0 , 1 ) \in \bar { \varphi } _ { 2 } \}$ . We calculate the intersection of the values of the vector k, i.e., 

$$
\begin{array}{l} \mathbb {S} _ {\cap} \\ = \mathbb {S} _ {0} \cap \mathbb {S} _ {1} \cap \mathbb {S} _ {2} \\ = \{(0, 0, 1, 1), (0, 1, 0, 1), (0, 1, 1, 0), (0, 1, 1, 1), (1, 0, 0, 1), \\ (1, 0, 1, 0), (1, 0, 1, 1), (1, 1, 0, 1), (1, 1, 1, 0), (1, 1, 1, 1) \} \\ \end{array}
$$

Thus, $( 0 , 0 , 1 , 0 )  ( 0 , 0 , 0 , 1 )$ is an valid division trail for L when the input vector $\boldsymbol { k } \in \mathbb { S } _ { \cap }$ . 

2. When the vector $\pmb { u } = ( 0 , 0 , 1 , 0 )$ , we calculate: 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u} = (0, 0, 1, 0)} (\boldsymbol {y}) = \bigoplus_ {y \in \mathbb {Y}} y _ {2} \\ = \bigoplus_ {x \in \mathbb {X}} x _ {2} \\ = \bigoplus_ {x \in \mathbb {X}} \pi_ {(0, 0, 1, 0)} (\boldsymbol {x}) \\ = 1 \\ \end{array}
$$

$\mathrm { i f } \oplus _ { x \in \mathbb { X } } \pi _ { ( 0 , 0 , 1 , 0 ) } ( x ) = 1$ , the vector ${ \pmb k } \in \mathbb { S } _ { 0 } = \mathbb { U } \backslash \{ { \bar { \varphi } } _ { 0 } ~ | ~ \varphi _ { 0 } = ( 0 , 0 , 1 , 0 ) ~ \forall ~$ $\langle { \bar { \varphi } } _ { 0 } \rangle$ . We calculate the intersection of the values of the vector k, i.e., 

$$
\begin{array}{l} \mathbb {S} _ {\cap} = \mathbb {S} _ {0} \\ = \{(0, 0, 0, 1), (0, 0, 1, 1), (0, 1, 0, 0), (0, 1, 0, 1), \\ (0, 1, 1, 0), (0, 1, 1, 1), (1, 0, 0, 0), (1, 0, 0, 1), \\ (1, 0, 1, 0), (1, 0, 1, 1), (1, 1, 0, 0), (1, 1, 0, 1), \\ (1, 1, 1, 0), (1, 1, 1, 1) \} \\ \end{array}
$$

Thus, $( 0 , 0 , 1 , 0 )  ( 0 , 0 , 0 , 1 )$ is an valid division trail for L when the input vector $\pmb { k } \in \mathbb { S } _ { \cap }$ . 

3. When the vector $\pmb { u } = ( 0 , 0 , 1 , 1 )$ , we calculate: 

$$
\begin{array}{l} \bigoplus_ {y \in \mathbb {Y}} \pi_ {\boldsymbol {u} = (0, 0, 1, 1)} (\boldsymbol {y}) \\ = \bigoplus_ {y \in \mathbb {Y}} y _ {2} y _ {3} \\ = \bigoplus_ {x \in \mathbb {X}} \left(x _ {0} x _ {1} x _ {2} \oplus x _ {2} \oplus x _ {2} x _ {3}\right) \\ = \bigoplus_ {x \in \mathbb {X}} \pi_ {(1, 1, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(0, 0, 1, 0)} (\boldsymbol {x}) \oplus \bigoplus_ {x \in \mathbb {X}} \pi_ {(0, 0, 1, 1)} (\boldsymbol {x}) \\ = (0 \text {   or   unknown   Depends   on   } \boldsymbol {k}) \oplus 1 \oplus (0 \text {   or   unknown   Depends   on   } \boldsymbol {k}) \\ \end{array}
$$

if $\bigoplus _ { x \in \mathbb { X } } \pi _ { ( 1 , 1 , 1 , 0 ) } ( x ) = 0$ , the vector ${ \pmb k } \in \mathbb { S } _ { 0 } = \mathbb { U } \backslash \{ \bar { \varphi } _ { 0 } \ | \ \varphi _ { 0 } = ( 1 , 1 , 1 , 0 ) \ \subseteq$ $\langle { \bar { \varphi } } _ { 0 } \rangle$ . Similarly, we can get $\pmb { k } \in \mathbb { S } _ { 1 } = \mathbb { U } \backslash \{ \bar { \varphi } _ { 1 } \ | \ \varphi _ { 1 } = ( 0 , 0 , 1 , 0 ) \ \succeq \bar { \varphi } _ { 1 } \}$ and $\pmb { k } \in \mathbb { S } _ { 2 } = \mathbb { U } \backslash \{ \bar { \varphi } _ { 2 } \mid \varphi _ { 2 } = ( 0 , 0 , 1 , 1 ) \in \bar { \varphi } _ { 2 } \}$ . We calculate the intersection of the values of the vector k, i.e., 

$$
\mathbb {S} _ {\cap} = \mathbb {S} _ {0} \cap \mathbb {S} _ {1} \cap \mathbb {S} _ {2} = \{(0, 1, 0, 1), (0, 1, 1, 1), (1, 0, 0, 1),
$$

$$
(1, 0, 1, 1), (1, 1, 0, 1), (1, 1, 1, 1) \}
$$

Thus, $( 0 , 0 , 1 , 0 )  ( 0 , 0 , 1 , 1 )$ is an valid division trail for L when the input vector $\pmb { k } \in \mathbb { S } _ { \cap }$ . 


Table 4: Correspondence between the vector u and $\pi _ { u } ( y )$ for the “S-box”


<table><tr><td>Vector <eq>\boldsymbol{u}</eq></td><td><eq>\pi_{\boldsymbol{u}}(\boldsymbol{y})</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 0, 0, 0]</eq></td><td>1</td></tr><tr><td><eq>\boldsymbol{u} = [0, 0, 0, 1]</eq></td><td><eq>y_3 = x_0x_1 \oplus x_2 \oplus x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 0, 1, 0]</eq></td><td><eq>y_2 = x_2</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 0, 1, 1]</eq></td><td><eq>y_2y_3 = x_0x_1x_2 \oplus x_2 \oplus x_2x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 1, 0, 0]</eq></td><td><eq>y_1 = x_1</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 1, 0, 1]</eq></td><td><eq>y_1y_3 = x_0x_1 \oplus x_1x_2 \oplus x_1x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 1, 1, 0]</eq></td><td><eq>y_1y_2 = x_1x_2</eq></td></tr><tr><td><eq>\boldsymbol{u} = [0, 1, 1, 1]</eq></td><td><eq>y_1y_2y_3 = x_0x_1x_2 \oplus x_1x_2 \oplus x_1x_2x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 0, 0, 0]</eq></td><td><eq>y_0 = x_0</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 0, 0, 1]</eq></td><td><eq>y_0y_3 = x_0x_1 \oplus x_0x_2 \oplus x_0x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 0, 1, 0]</eq></td><td><eq>y_0y_2 = x_0x_2</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 0, 1, 1]</eq></td><td><eq>y_0y_2y_3 = x_0x_1x_2 \oplus x_0x_2 \oplus x_0x_2x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 1, 0, 0]</eq></td><td><eq>y_0y_1 = x_0x_1</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 1, 0, 1]</eq></td><td><eq>y_0y_1y_3 = x_0x_1 \oplus x_0x_1x_2 \oplus x_0x_1x_3</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 1, 1, 0]</eq></td><td><eq>y_0y_1y_2 = x_0x_1x_2</eq></td></tr><tr><td><eq>\boldsymbol{u} = [1, 1, 1, 1]</eq></td><td><eq>y_0y_1y_2y_3 = x_0x_1x_2x_3</eq></td></tr></table>

# C Division trail for L of Present S-box

Table 5 presents the division trails for L of Present S-box. 

# D Linear inequalities description BDPT of the Extension-S-box

The following inequalities are the 6 inequalities used to describe the “S-box” whose feasible solutions are exactly the 26 division trails for K of the “S-box” where $( a _ { 0 } , a _ { 1 } , a _ { 2 } , a _ { 3 } )  ( b _ { 0 } , b _ { 1 } , b _ { 2 } , b _ { 3 } )$ denotes a division trail. 

$$
\mathcal {O} = \left\{ \begin{array}{l} - a _ {0} - a _ {2} - a _ {3} + b _ {0} + b _ {2} + b _ {3} \geq 0 \\ - a _ {1} - a _ {2} - a _ {3} + b _ {1} + b _ {2} + b _ {3} \geq 0 \\ a _ {2} + a _ {3} - b _ {0} - b _ {2} - b _ {3} + 1 \geq 0 \\ a _ {2} + a _ {3} - b _ {1} - b _ {2} - b _ {3} + 1 \geq 0 \\ a _ {0} + a _ {1} + a _ {2} + a _ {3} - b _ {0} - b _ {1} - b _ {2} - b _ {3} \geq 0 \\ a _ {2} - b _ {2} \geq 0 \\ a _ {0}, a _ {1}, a _ {2}, a _ {3}, b _ {0}, b _ {1}, b _ {2}, b _ {3} \text { are   binaries } \end{array} \right. \tag {3}
$$

The following inequalities are the 10 inequalities used to describe the “S-box” whose feasible solutions are exactly the 30 division trails for L of the “S-box” 


Table 5: Division trails for L of Present S-box


<table><tr><td>Input <eq>\ell</eq></td><td>Output <eq>\mathbb{L}</eq></td></tr><tr><td><eq>[0, 0, 0, 0]</eq></td><td><eq>\{[0, 0, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 0, 0, 1]</eq></td><td><eq>\{[0, 0, 0, 1], [0, 1, 0, 1], [1, 0, 0, 0], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 0, 1, 0]</eq></td><td><eq>\{[0, 0, 1, 0], [0, 1, 1, 0], [1, 0, 0, 0], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 0, 1, 1]</eq></td><td><eq>\{[0, 0, 1, 1], [0, 1, 0, 0], [0, 1, 0, 1], [0, 1, 1, 0], [1, 0, 0, 1], [1, 0, 1, 0], [1, 0, 1, 1], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 1, 0, 0]</eq></td><td><eq>\{[0, 0, 0, 1], [0, 1, 0, 0], [1, 0, 0, 1], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 1, 0, 1]</eq></td><td><eq>\{[0, 1, 0, 1], [1, 0, 0, 1], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 1, 1, 0]</eq></td><td><eq>\{[0, 0, 0, 1], [0, 1, 1, 0], [1, 0, 0, 0], [1, 0, 0, 1], [1, 0, 1, 0], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[0, 1, 1, 1]</eq></td><td><eq>\{[0, 0, 1, 0], [0, 0, 1, 1], [0, 1, 1, 0], [1, 0, 0, 0], [1, 0, 0, 1], [1, 0, 1, 1], [1, 1, 0, 1]\}</eq></td></tr><tr><td><eq>[1, 0, 0, 0]</eq></td><td><eq>\{[0, 0, 0, 1], [0, 0, 1, 0], [0, 0, 1, 1], [0, 1, 0, 0], [1, 0, 0, 0], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[1, 0, 0, 1]</eq></td><td><eq>\{[0, 0, 1, 1], [0, 1, 0, 0], [0, 1, 0, 1], [0, 1, 1, 0], [1, 0, 1, 0], [1, 1, 1, 0]\}</eq></td></tr><tr><td><eq>[1, 0, 1, 0]</eq></td><td><eq>\{[0, 0, 1, 0], [0, 1, 0, 0], [0, 1, 0, 1], [0, 1, 1, 1], [1, 0, 0, 1], [1, 0, 1, 0], [1, 0, 1, 1], [1, 1, 0, 1], [1, 1, 1, 0]\}</eq></td></tr><tr><td><eq>[1, 0, 1, 1]</eq></td><td><eq>\{[0, 0, 1, 0], [0, 0, 1, 1], [0, 1, 0, 0], [0, 1, 1, 0], [0, 1, 1, 1], [1, 0, 0, 0], [1, 0, 1, 0], [1, 1, 0, 0], [1, 1, 0, 1]\}</eq></td></tr><tr><td><eq>[1, 1, 0, 0]</eq></td><td><eq>\{[0, 0, 1, 0], [0, 0, 1, 1], [1, 0, 0, 1], [1, 1, 0, 0]\}</eq></td></tr><tr><td><eq>[1, 1, 0, 1]</eq></td><td><eq>\{[0, 0, 1, 0], [0, 1, 0, 0], [0, 1, 1, 1], [1, 0, 0, 0], [1, 0, 0, 1], [1, 0, 1, 0], [1, 1, 1, 0]\}</eq></td></tr><tr><td><eq>[1, 1, 1, 0]</eq></td><td><eq>\{[0, 1, 0, 1], [0, 1, 1, 1], [1, 0, 1, 1], [1, 1, 0, 1], [1, 1, 1, 0]\}</eq></td></tr><tr><td><eq>[1, 1, 1, 1]</eq></td><td><eq>\{[1, 1, 1, 1]\}</eq></td></tr></table>

where $( a _ { 0 } , a _ { 1 } , a _ { 2 } , a _ { 3 } )  ( b _ { 0 } , b _ { 1 } , b _ { 2 } , b _ { 3 } )$ denotes a division trail. 

$$
\mathcal {O} = \left\{ \begin{array}{l} - a _ {1} - a _ {2} - a _ {3} + b _ {1} + b _ {2} + b _ {3} \geq 0 \\ a _ {0} - b _ {0} \geq 0 \\ a _ {1} - b _ {1} \geq 0 \\ - a _ {0} - a _ {2} - a _ {3} + b _ {0} + b _ {2} + b _ {3} \geq 0 \\ a _ {2} - b _ {2} \geq 0 \\ a _ {0} - a _ {1} + b _ {1} \geq 0 \\ a _ {0} + a _ {2} + a _ {3} - b _ {3} \geq 0 \\ - a _ {0} + a _ {1} + b _ {0} \geq 0 \\ a _ {1} + a _ {2} + a _ {3} - b _ {3} \geq 0 \\ a _ {3} - b _ {0} - b _ {1} - b _ {2} - b _ {3} + 3 \geq 0 \\ a _ {0}, a _ {1}, a _ {2}, a _ {3}, b _ {0}, b _ {1}, b _ {2}, b _ {3} \text { are   binaries } \end{array} \right. \tag {4}
$$

# E Proof of Propositon 1

According to Definition 4, we have the following iteration expression 

$$
\mathbb {K} _ {i} = f _ {e} (\mathbb {K} _ {i - 1}) \cup f _ {k} (\mathbb {L} _ {i}) = f _ {e} (\mathbb {K} _ {i - 1}) \cup f _ {k} \circ f _ {e} (\mathbb {L} _ {i - 1})
$$

$$
\mathbb {L} _ {i} = f _ {e} (\mathbb {L} _ {i - 1})
$$

Thus, 

$$
\begin{array}{l} \mathbb {K} _ {r} = f _ {e} (\mathbb {K} _ {r - 1}) \cup f _ {k} (\mathbb {L} _ {r}) \\ = f _ {e} \left(f _ {e} (\mathbb {K} _ {r - 2}) \cup f _ {k} (\mathbb {L} _ {r - 1})\right) \cup f _ {k} (\mathbb {L} _ {r}) \\ = f _ {e} \circ f _ {e} (\mathbb {K} _ {r - 2}) \cup f _ {e} \circ f _ {k} (\mathbb {L} _ {r - 1}) \cup f _ {k} (\mathbb {L} _ {r}) \\ \begin{array}{c} \bullet \\ \vdots \\ \bullet \end{array} \\ = \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {k}) \cup \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r - 1} \circ f _ {k} \circ f _ {e} (\boldsymbol {\ell}) \cup \dots \cup f _ {k} \circ \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {\ell}) \\ \mathbb {L} _ {r} = \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\ell) \\ \end{array}
$$

$$
\begin{array}{l} \begin{array}{c} \bullet \\ \vdots \\ \bullet \end{array} \\ = \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {k}) \cup \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r - 1} \circ f _ {k} \circ f _ {e} (\boldsymbol {\ell}) \cup \dots \cup f _ {k} \circ \underbrace {f _ {e} \circ \cdots \circ f _ {e}} _ {r} (\boldsymbol {\ell}) \\ \end{array}
$$

# F Integral Distinguishers listed in Table 1

For Simon and Simeck family block ciphers, all the integral distinguishers can be extended one more round by the technique in [22]. Moreover, since there is no whitening key at the beginning, we can trivially extend the integral distinguisher of GIFT-64 by one round [1]. 

# F.1 SIMON32’s 14-round Distinguisher

Input:(caaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaa) 

Output:(????????????????, ?0??????0??????0) 

# F.2 SIMON48’s 15-round Distinguisher

Input: (caaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaa) 

Output: (????????????????????????, 000000000000000000000000) 

# F.3 SIMON64’s 17-round Distinguisher

$$
\begin{array}{l} \text { Input: } \left( \begin{array}{c} \text { caaaaaaaacaaaaaataaaataaaataaaataaaataaa , } \\ \text { aaaaaaaaacaaaaaataaaataaaataaaataaaataaaataaa . } \end{array} \right) \\ \text {Output:} \left( \begin{array}{c} \text {????????????????????????????????????}, \\ 0 0 0 0 0 0 0 0 0 0 0 0 0 0? 0 0???? 0 0 0 0 0 0 0 0 0 0) \end{array} \right. \\ \end{array}
$$

# F.4 SIMON96’s 21-round Distinguisher

```txt
Input: (caaaaacaaaaacaaaaacaaaaacaaaaacaaaaacaaaaacaaaaacaaaaa, aaaaaacaaaaacaaaaacaaaaacaaaaacaaaaacaaaaacaaaaa)  
Output: (??????????????????????????????????????????????????, 0?0????0?????????????????????????????????????????????????0?????0?) 
```

# F.5 SIMON128’s 25-round Distinguisher

```txt
Input: ( caaaaaaaaacaaaaaaacaaaaaaacaaaaaa
    aaaaaaaaaaaacaaaaaacaaaaaacaaaaaacaaaaaa,
    aaaaaaaaaaaacaaaaaacaaaaaacaaaaaacaaaaaa
    aaaaaaaaaaaacaaaaaacaaaaaacaaaaaacaaaaaa
    ??????????????????????????????????
Output: ( ??????????????????????????????????,
    0?0?????????????????????????????????
    ???????????????????????????????????0?) 
```

# F.6 SIMECK32’s 14-round Distinguisher

```txt
Input:(caaaaaaaaaaaaaaaa, aaaaaaataaaaaaaa)
Output:???????????????, 00???00???00???0) 
```

# F.7 SIMECK48’s 17-round Distinguisher

```txt
Input: (caaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaa)
Output: (???????????????????????, 0???00??????????????00???) 
```

# F.8 SIMECK64’s 20-round Distinguisher

```txt
Input: (caaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa)
Output: (????????????????????????????????, 00???0????????????????????????0???0) 
```

# F.9 PRESENT’s 9-round Distinguisher

```txt
Input: (aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac),  
Output: (???0???0???00000???0???0???00000,  
(???0???0???00000???0???0???00000) 
```

# F.10 RECTANGLE’s 10-round Distinguisher

$\left( \begin{array}{l}\texttt{aaaaaaaac} \\ \texttt{aaaaaaaaaa} \\ \texttt{aaaaaaa} \\ \texttt{aaaaaaa} \end{array} \right)\to \left( \begin{array}{l}?0??00000???00?0 \\ ???????????????? \\ ?????????????????? \\ ?????????????????? \end{array} \right)$ 

# F.11 GIFT-64’s 10-round Distinguisher

```txt
Input: (aaaaaaaacaaaaaaacaaaaaacaaaaaacaaa, aaaaaaaaacaaaaaacaaaaaacaaaaaacaaa)  
Output: (???b???b???b???b???b???b???b???b, ???b???b???b???b???b???b???b???b) 
```

# F.12 Integral Distinguishers of SIMON(102)

In [10], another variant of Simon family named Simon(102) is proposed with rotation constants (1,0,2). Hu et al. [8] proposed a variant BDPT and applied it to improve the integral distinguishers of Simon(102). The results are shown in F.13–F.15, where ‘*’ represents that the output bit is ‘0’ or ‘1’. 

# F.13 SIMON(102) 32’s 14-round Distinguisher in [8]

```txt
Input:(caaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaa)
Output:(?????????????????, 0*?????????????*) 
```

# F.14 SIMON(102) 48’s 15-round Distinguisher in [8]

```css
Input: (caaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaa)
Output: (???????????????????????, 0*?????????????????????*) 
```

# F.15 SIMON(102) 64’s 17-round Distinguisher in [8]

```txt
Input: (caaaaaaaaacaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa)
Output: (????????????????????????????????, 0*?????????????????????????????*) 
```

Determining ‘*’ is ‘0’ or ‘1’ can be helpful to integral attacks on Simon(102). Therefore, we apply Algorithm 3 and 4 to search integral distinguishers of Simon(102) based on BDPT, and obtain more accurate integral distinguishers compared with [8]. The results are shown in F.16–F.18. Besides, these integral distinguishers can be obtained by the method of exploring secret keys in [25]. Note that our automatic search model supposes that all secret keys are chosen randomly. If consider the secret keys, we may obtain better integral distinguishers. 

# F.16 SIMON(102) 32’s 14-round Distinguisher

```txt
Input:(caaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaa)
Output:?????????????????, 01??????????????1) 
```

# F.17 SIMON(102) 48’s 15-round Distinguisher

Input: (caaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaa) 

Output: (????????????????????????, ????????????????????????) 

# F.18 SIMON(102) 64’s 17-round Distinguisher

```txt
Input: (caaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa)
Output: (??????????????????????????????), 01??????????????????????????????????1) 
```