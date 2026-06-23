# On MILP-Based Automatic Search for Bit-Based Division Property for Ciphers with (Large) Linear Layers

Muhammad ElSheikh and Amr M. Youssef(B) 

Concordia Institute for Information Systems Engineering, Concordia University, Montr´eal, QC, Canada 

{m elshei,youssef}@ciise.concordia.ca 

Abstract. With the introduction of the division trail, the bit-based division property (BDP) has become the most efficient method to search for integral distinguishers. The notation of the division trail allows us to automate the search process by modelling the propagation of the DBP as a set of constraints that can be solved using generic Mixed-integer linear programming (MILP) and SMT/SAT solvers. The current models for the basic operations and Sboxes are efficient and accurate. In contrast, the two approaches to model the propagation of the BDP for the non-bitpermutation linear layer are either inaccurate or inefficient. The first approach relies on decomposing the matrix multiplication of the linear layer into COPY and XOR operations. The model obtained by this approach is efficient, in terms of the number of the constraints, but it is not accurate and might add invalid division trails to the search space, which might lead to missing the balanced property of some bits. The second approach employs a one-to-one map between the valid division trails through the primitive matrix represented the linear layer and its invertible submatrices. Despite the fact that the current model obtained by this approach is accurate, it is inefficient, i.e., it produces a large number of constraints for large linear layers like the one of Kuznyechik. In this paper, we address this problem by utilizing the one-to-one map to propose a new MILP model and a search procedure for large non-bit-permutation layers. As a proof of the effectiveness of our approach, we improve the previous 3- and 4-round integral distinguishers of Kuznyechik and the 4-round one of PHOTON’s internal permutation (P288). We also report, for the fist time, a 4-round integral distinguisher for Kalyna block cipher and a 5-round integral distinguisher for PHOTON’s internal permutation $\left( P _ { 2 8 8 } \right)$ . 

Keywords: Bit-based division property Integral Linear layer MILP  Kuznyechik  Kalyna  PHOTON 

# 1 Introduction

The division property is a generalized integral property that exploits the algebraic degree of the nonlinear components of block ciphers [16]. Since it was proposed by Todo at Eurocrypt 2015, it has become one of the most efficient methods to build integral distinguishers. It has been used to analyze the security claims of many symmetric-key primitives, e.g., the full round MISTY1 is broken using a 6-round integral distinguisher found by the division property [17]. The division property was succeeded by a more precise version called the bit-based division property (BDP) in [18] which exploits the internal structure of the nonlinear components to analyzes block ciphers at the bit level. Even though the BDP is more accurate and can find better integral distinguishers, handling its propagation is computationally intensive. The first search tool utilized the bitbased division property was limited to building integral distinguishers for block ciphers with block size less than 32 bits since the complexity of the search is around (2n) where n is the block size [18]. 

Xiang et al. [19] have overcome the problem of the restriction on the block size by proposing the division trails. Using the division trial, the search process for an integral distinguisher can be converted to checking whether a specific division trail exists or not. They also proposed a systematic method to model the propagation rules of the BDP as a set of linear constraints. Hence, the search process can be efficiently automated with the help of generic Mixed Integer Linear Programming (MILP) and SAT solvers. Moreover, Xiang et al. provided an accurate model for the propagation of the BDP through the basic operations; COPY, XOR, and AND, in addition to an accurate model for Sboxes. With the help of these models, it is now feasible to look for integral distinguishers for many ciphers that utilize these operations when the used linear layer is a bit-permutation. 

For ciphers with non-bit-permutation linear layers, Sun et al. [15] proposed a model relying on decomposing the matrix corresponding to the linear layer into its basic operations; COPY and XOR. We refer to this model through our paper as Disjointed Representation and we will provide more details about it in the following sections. The main two advantages of this model are: (i) it is applicable to all kinds of linear layers, and (ii) the number of constraints needed to model the propagation of the BDP is small, precisely, 2n where n denotes the size of the matrix input in bits. However, this representation does not model the propagation accurately and might add invalid division trails to the search space which might lead to missing the balanced property of some bits. 

Another model for the propagation of the BDP through non-bit-permutation linear layers is presented by Zhang and Rijmen in [20]. They observed that there is a one-to-one map between each valid division trail and one of the invertible sub-matrices of the matrix, M , representing the linear layer. They were able to convert this map to a set of MILP constraints. Unlike the first model provided by [15], the new model is more accurate. However, the number of the MILP constraints grows exponentially with the size of M . Recently, Hu et al. partially solved this problem in [10] by utilizing the one-to-one relation to build a model of 4-degree constraints that can be solved using SMT/SAT. The new number of the constraints is proportional to the square of matrix size. Unfortunately, this model is still not suitable for some large linear layers such as the one of Kuznyechik [3]. 

Our Contributions. In this paper, we propose a new model for the propagation of the BDP through large linear layers. In particular, we utilize the same one-to-one map proposed by Zhang and Rijmen to derive a set of constraints that filter out all non-invertible sub-matrices, part of them during the offline modelling process and the other part on-the-fly during the search process. In order to validate the correctness of our approach, we use our model to reproduce the results of the 4- and 5-round key-dependent integral distinguishers of AES reported in [10]. With the help of our model, we improved the previous 3- and 4-round integral distinguishers of Kuznyechik block cipher and the 4-round one of PHOTON’s internal permutation $( P _ { 2 8 8 } )$ . We also report, for the fist time, a 4-round integral distinguisher for Kalyna block cipher [13] and a 5-round integral distinguisher for PHOTON’s internal permutation $( P _ { 2 8 8 } )$ [8]. Table 1 summarizes our results. 


Table 1. Integral distinguishers for Kuznyechik, Kalyna and PHOTON.


<table><tr><td>Ciphers</td><td>#Rounds</td><td>log2(Data)</td><td>Reference</td></tr><tr><td rowspan="4">Kuznyechik</td><td>3</td><td>117*</td><td>[1]</td></tr><tr><td>3</td><td>56</td><td>Sect. 5.1</td></tr><tr><td>4</td><td>127*</td><td>[1]</td></tr><tr><td>4</td><td>120</td><td>Sect. 5.1</td></tr><tr><td rowspan="3">Kalyna-128</td><td>4†</td><td>64</td><td>Sect. 5.2</td></tr><tr><td>4§</td><td>96</td><td>Sect. 5.2</td></tr><tr><td>4‡</td><td>62</td><td>Sect. 5.2</td></tr><tr><td rowspan="3">PHOTON (P288)</td><td>4</td><td>48</td><td>[15]</td></tr><tr><td>4</td><td>40</td><td>Sect. 5.3</td></tr><tr><td>5</td><td>280</td><td>Sect. 5.3</td></tr></table>

- Higher-order differential. 

† Without pre-whitening operation. 

§ With pre-whitening operation. 

‡ A key-dependent distinguisher which depends on the 

32 least significant bits of the pre-whitening key. 

Outline. The rest of this paper is organized as follows. In Sect. 2 we recall some relevant definitions and revisit the MILP model for the basic operations. In Sect. 3, we revisit the previous MILP models for the linear layers. Next, we illustrate in details our new model and search approach in Sect. 4. In Sect. 5, we show some applications of the new model. Finally, the paper is concluded in Sect. 6. 

# 2 Preliminaries

# 2.1 Notations and Definitions

We represent n-bit vectors using bold letters, $e . g . , \pmb { u } \in \mathbb { F } _ { 2 } ^ { n }$ . The i-th element of u is expressed as $u _ { i }$ and the Hamming weight $h w ( { \pmb u } )$ is calculated as $h w ( { \pmb u } ) =$ $\textstyle \sum _ { i = 0 } ^ { n - 1 } u _ { i }$ . For a matrix $M \in \mathbb { F } _ { 2 } ^ { p \times q }$ , we use the notation $M ( i , j )$ to represent the element of M located at the i-th row and j-th column, $r _ { i } = M ( i , * )$ to represent the i-th row, and $c _ { j } = M ( * , j )$ to represent the j-th column of M . Given two ∗q-bit and p-bit vectors u and v, we define Mv,u ∈ Fhw(v)×hw(u)2 as a sub-matrix $M _ { \pmb { v } , \pmb { u } } \in \mathbb { F } _ { 2 } ^ { h w ( \pmb { v } ) \times h w ( \pmb { u } ) }$ of M as follows 

$$
M _ {\boldsymbol {v}, \boldsymbol {u}} = [ M (i, j) ], s. t. v _ {i} = u _ {j} = 1, \forall 0 \leq i \leq p - 1, 0 \leq j \leq q - 1
$$

Given a q-bit vector u, we define $M _ { \pmb { u } } \in \mathbb { F } _ { 2 } ^ { p \times h w ( \pmb { u } ) }$ as a sub-matrix of M as follows 

$$
M _ {\boldsymbol {u}} = [ M (*, j) ], s. t. u _ {j} = 1, \forall 0 \leq j \leq q - 1
$$

Definition 1 (Division Trail [19]). Let f denote the round function of an iterated block cipher. Assume that the input multiset to the block cipher has the initial division property propagation through f b ${ \mathcal { D } } _ { \{ k \} } ^ { n }$ and denote the division property after i-round. Thus, we have the following chain of division $\ddot { \mathcal { D } } _ { \mathbb { K } _ { i } } ^ { \lambda }$ property propagations: $\{ k \} \ { \stackrel { \mathrm { d e f } } { = } } \ \mathbb { K } _ { 0 } \ { \stackrel { f } { \to } } \ \mathbb { K } _ { 1 } \ { \stackrel { f } { \to } } \ \mathbb { K } _ { 2 } \ { \stackrel { f } { \to } } \ \cdot \ \cdot \ \stackrel { f } { \to } \ \mathbb { K } _ { r }$ . Moreover, for any vector $\pmb { k } _ { i } ^ { * } \in \mathbb { K } _ { i } ( i \geq 1 )$ , there must exist a vector $\pmb { k } _ { i - 1 } ^ { * } \in \mathbb { K } _ { i - 1 }$ such that $k _ { i - 1 } ^ { * }$ can propagate to $\boldsymbol { k } _ { i } ^ { * }$ by the division property propagation rules. Furthermore, for $( \pmb { k } _ { 0 } , \pmb { k } _ { 1 } , \dots , \pmb { k } _ { r } ) \in \mathbb { K } _ { 0 } \times \mathbb { K } _ { 1 } \times \dots \times \mathbb { K } _ { r } , \ i f \ \pmb { k } _ { i - 1 }$ can propagate to $\boldsymbol { k } _ { i }$ for all $i \in \{ 1 , 2 , \ldots , r \}$ , we call $( k _ { 0 } , k _ { 1 } , \ldots , k _ { r } )$ an r-round division trail. 

# 2.2 MILP-Based Automated Search for Bit-Based Division Property

As we mentioned above, the first automated search tool for the bit-based division property was limited to building integral distinguishers for block ciphers with block size less than 32 bits [18]. Then, Xiang et al. [19] proposed the division trails to solve this problem. In particular, with the help of the division trial, the search process for an integral distinguisher is converted to checking if the division trail $k _ { 0 }  \cdots  e _ { i }$ (a unit vector whose i-th element is 1) does exist or not. If it does not exist, then the i-th bit of r-round output is balanced. 

In the following, we summarize the MILP constraints that are used to model the propagation rules of the bit-based division property through the basic operations in block ciphers. For more details, we refer the reader to [4, 6, 15, 19]. 

– Model for COPY: Let $( a ) \xrightarrow { \mathsf { C O P Y } } \left( b _ { 1 } , b _ { 2 } , \ldots , b _ { m } \right)$ denote the division trail through COPY function, where a single bit (a) is copied to m bits. Then, it can be described using the following MILP constraints: 

$$
a - b _ {1} - b _ {2} - \dots - b _ {m} = 0, \text {   where   } a, b _ {1}, b _ {2}, \ldots , b _ {m} \text {   are   binary   variables.   }
$$

– Model for XOR: Let $( a _ { 1 } , a _ { 2 } , \ldots , a _ { m } ) \xrightarrow { \mathtt { X O R } }$ (b) denote the division trail through an XOR function, where m bits are compressed to a single bit (b) using an XOR operation. Then, it can be described using the following MILP constraints: 

$$
a _ {1} + a _ {2} + \dots + a _ {m} - b = 0, \text {   where   } a _ {1}, a _ {2}, \ldots , a _ {m}, b \text {   are   binary   variables. }
$$

– Model for Sboxes: The bit-based division property introduced in [18] is limited to bit-orientated ciphers and cannot be applied to ciphers with Sboxes. Xiang et al. [19] complemented this work by proposing an algorithm to accurately compute the bit-based division property through an Sbox. Briefly, they represented the Sbox using its algebraic normal form (ANF). Then, the division trail though an n-bit Sbox can be represented as a set of 2n-dimensional binary vectors $\in \{ 0 , 1 \} ^ { 2 n }$ which has a convex hull. The H-Representation of this convex hull can be computed using readily available functions such as inequality generator() function in SageMath1 which returns a set of linear inequalities that describe these vectors. We use this set of inequalities as MILP constraints to present the division trail though the Sbox. 

# 3 Previous MILP-Based Modelling for Linear Layers

The propagation of the bit-based division property through bit-permutation linear layers, e.g., the linear layer of PRESENT [2], can be easily modelled by rearranging the variables based on the permutation. In contrast, the non-bitpermutation linear layers, e.g., the linear layers of AES and Kuznyechik [3], needs a more complex model. 

In this section, we revisit the two methods used to model the propagation of the BDP through non-bit-permutation linear layers. These methods relay on representing the matrix multiplication in the linear layer at the bit level. Suppose the linear layer can be represented as a matrix multiplication over the field $\mathbb { F } _ { 2 ^ { m } }$ using the matrix $\boldsymbol { M } ^ { \prime } \in \mathbb { F } _ { 2 ^ { m } } ^ { s \times s }$ . Given the irreducible polynomial of the field $\mathbb { F } _ { 2 ^ { m } }$ , ∈ 2 we can derive a unique equivalent matrix $M \in \mathbb { F } _ { 2 } ^ { n \times n }$ called the primitive matrix at the bit level where $n = s \times m$ . 

# 3.1 Disjointed Representation

Since the primitive matrix M is presented at the bit level, i.e., $M ( i , j ) \in \{ 0 , 1 \}$ , we can decompose the linear layer into its basic operations, i.e., AND with 0 or 1 and XOR operations. Consequently, the propagation of the BDP can be easily modelled using the models of the basic operations [15]. 

Let $\pmb { u } \xrightarrow { M } \pmb { v }$ denote the division trail through the linear layer where u, $\pmb { v } \in \mathbb { F } _ { 2 } ^ { n }$ . By defining a set of auxiliary binary variables $\pmb { t } = \{ t _ { ( i , j ) }$ if $M ( i , j ) = 1 , 0 \leq i , j \leq$ $n - 1 \}$ , we can model the propagation of the BDP at the bit level in two steps as follows: 

Hence, the total number of constraints $\# { \mathcal { L } } = 2 n$ . 

Limitations. Despite the fact that this method is simple and efficient in terms of the number of constraints, it cannot handle the cancellation between monomials since it handles each output bit individually. Hence, it is not precise and it might produce invalid division trails leading to missing the balanced property of some output bits. For further details, see [20]. 

# 3.2 Compact Representation

One method to overcome the problem of the monomial cancellations is to deal with the linear layer as a one single block like an S-box. However, the large size of the linear layer renders this approach computationally infeasible in many cases. 

In this context, Zhang and Rijmen observed that there is a one-to-one map between the accurate division trails of the primitive matrix M and invertible sub-matrices of M [20]. This observation is stated in the following theorem. 

Theorem 1 ([20]). Let M be the $n \times n$ primitive matrix of an invertible linear transformation and u, $\pmb { v } \in \mathbb { F } _ { 2 } ^ { n }$ . Then $u \xrightarrow { M } v$ is one of the valid division trails of the linear transform M if and only if $M _ { v , u }$ is invertible. 

Using this one-to-one map, they proposed a systematic method to model a binary matrix $\boldsymbol { M } ^ { \prime } \in \mathbb { F } _ { 2 ^ { m } } ^ { s \times s }$ as a set of MILP constraints. For more derails, see [20]. In this case, the total number of constraints $\# \mathcal { L } = m \times ( 2 ^ { s } - 1 )$ . 

Regarding the non-binary matrices, we can still use the same method, but the number of constrains will exponentially increase with the size of the primitive matrix, i.e., if the primitive matrix M is $n \times n$ , then the total number of constraints # ${ \mathcal { L } } = 2 ^ { n } - 1$ . 

Hu et al. presented an updated version of Theorem 1 in [10]. They removed the restriction that the primitive matrix M must be invertible to have valid division trails. Consequently, the primitive matrix M could be in general of size $p \times q$ . Hence, u $\xrightarrow { M } v$ is one of the valid division trails of M if and only if $M _ { v , u }$ is invertible where u and v are $q -$ and p-bit vectors, and $h w ( \pmb { u } ) = h w ( \pmb { v } )$ . Hu et al. also utilized this one-to-one map to present a new model for the propagation of the BDP through a non-binary matrix using less number of constraints. If a primitive matrix M is $p \times q$ , then the total number of constraints will be $\# { \mathcal { L } } = p ^ { 2 }$ . It should be mentioned that the constraints are 4-degree ones, therefore it is solvable using SMT/SAT solvers and cannot be handled using MILP solvers. For more details, see [10]. 

Limitations. Even though the models by Zhang and Rijmen, and Hu et $a l .$ are accurate, they are inefficient for large linear layers, $e . g .$ , the primitive matrix corresponding to the linear layer of Kuznyechik is $1 2 8 \times 1 2 8$ , therefore we will need $2 ^ { 1 2 8 }$ or $\bar { 1 2 8 } ^ { 2 } = 2 ^ { 1 4 }$ constraints to model a single linear layer if we use Zhang and Rijmen and Hu et al. methods receptively. Therefore, when the distinguisher covers many rounds, it will be computationally infeasible for current MILP/SAT solvers to handle the model due to the large number of the constraints. 

# 4 MILP-Based Modelling for (Large) Linear Layers

As mentioned in the previous section, the current models for the non-bitpermutation linear layer in the literature are either inaccurate or inefficient for large linear layers. In this paper, we tackle this problem by proposing an accurate model for the linear layer when its input division property is priorly known before the modelling step. Thereby, this model is more suitable for the first round of the distinguisher. Regarding the other rounds of the distinguisher when the input division property cannot be determined during the modelling, we use the disjointed representation described in Sect. 3.1 and address its inaccuracy by discarding any invalid trails on-fly during the search process. 

# 4.1 Prior-Known Input Division Property to the Linear Layer

Suppose the primitive matrix M is of size $p \times q$ and let u be the input division property to M and assume it is determined a priori. Consequently, we can utilize Theorem 1 and its updated version in [10] to derive all correct division trails. The naive method to do so is by exhaustively trying all the values of the output division property v such that $h w ( \pmb { u } ) = h w ( \pmb { v } )$ and checking if the sub-matrix $M _ { v , u }$ is invertible. Despite the correctness of this method, we need to try $\binom { p } { h w ( { \pmb u } ) }$ sub-matrices which is a very large number in almost all the cases. Moreover, we have to find a method to encode these division trails as MILP constraints to build a large model that covers many number of rounds. In the following, we explain our main idea to overcome this problem. 

Main Idea. Based on Theorem 1, the sub-matrix $M _ { v , u }$ must be invertible to have a valid trail u $\xrightarrow { M } { \pmb { v } } , i . e .$ , the sub-matrix $M _ { v , u }$ must not include linearly dependent rows. Given the input division property u, we can construct the column matrix $M _ { u }$ . Subsequently, we can get the row echelon form of $M _ { u }$ using the Gaussian eliminations, and obtain all the sets of linearly dependent rows. Then, instead of checking each value of v (as in the naive method), we derive a set of constraints that guarantee the bits $v _ { i }$ do not lead to including any set of linearly dependent rows from $M _ { u }$ . In order to complete the model, one more constraint should be added to enforce $h w ( \pmb { u } ) = h w ( \pmb { v } )$ . Hence, the value of v that satisfies these constraints is indeed a valid output division property. 

The following examples illustrates our idea. 

Detailed Example. Assume a toy linear layer where its primitive matrix M is $8 \times 8$ . Given the input division property $\pmb { u } = ( 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 )$ , we can construct the column matrix $M _ { u }$ by choosing the columns of M that correspond to the nonzero bits in u. 

$$
M = \left[ \begin{array}{l l l l l l l l} 1 & 0 & 0 & 0 & 0 & 0 & 0 & 0 \\ 1 & 1 & 0 & 0 & 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 & 0 & 0 & 0 & 0 \\ 0 & 0 & 1 & 0 & 0 & 0 & 0 & 0 \\ 0 & 0 & 0 & 1 & 0 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 & 1 & 1 & 0 & 0 \\ 1 & 1 & 0 & 0 & 0 & 0 & 1 & 0 \\ 0 & 0 & 1 & 1 & 0 & 0 & 0 & 1 \end{array} \right] \xrightarrow {\boldsymbol {u}} M _ {\boldsymbol {u}} = \left[ \begin{array}{l l l l l} 1 & 0 & 0 & 0 & 0 \\ 1 & 1 & 0 & 0 & 1 \\ 0 & 1 & 0 & 0 & 0 \\ 0 & 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & 1 & 0 \\ 0 & 1 & 0 & 0 & 1 \\ 1 & 1 & 0 & 0 & 0 \\ 0 & 0 & 1 & 1 & 0 \end{array} \right]
$$

We follow the procedure given below to derive a set of linear constraints as a function in the output division property $\pmb { v } = ( v _ { 0 } , v _ { 1 } , v _ { 2 } , v _ { 3 } , v _ { 4 } , v _ { 5 } , v _ { 6 } , v _ { 7 } )$ to trace the propagation of the division property for $M _ { u }$ . 

1. Check whether ${ R a n k } ( M _ { \mathbf { u } } ) = h w ( \pmb { u } )$ to ensure that there is at least one full rank (invertible) sub-matrix, and hence at least one valid division trail. Otherwise, we conclude that u cannot be propagated to any valid v. 

2. Use Gaussian eliminations to put $M _ { u }$ in its row echelon form while keeping track the row operations. Hence, each all-zero row in the row echelon form implies a set of linearly dependent rows in the original matrix $M _ { \pmb { u } } , \ e . \ g .$ , the first all-zero row in our example can be expressed as $r _ { 0 } + r _ { 1 } + r _ { 5 } = 0$ which means that the rows $\{ r _ { 0 } , r _ { 1 } , r _ { 5 } \}$ from $M _ { u }$ are linearly dependent. The details of the Gaussian elimination steps for our example can be found in [7]. 

$$
\left[ \begin{array}{c c c c c} 1 & 0 & 0 & 0 & 0 \\ 1 & 1 & 0 & 0 & 1 \\ 0 & 1 & 0 & 0 & 0 \\ 0 & 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & 1 & 0 \\ 0 & 1 & 0 & 0 & 1 \\ 1 & 1 & 0 & 0 & 0 \\ 0 & 0 & 1 & 1 & 0 \end{array} \right] \xrightarrow [ E l i m i n a t i o n ]{\text {Gaussian}} \left[ \begin{array}{c c c c c} 1 & 0 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 & 1 \\ 0 & 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 0 & 1 \\ 0 & 0 & 0 & 0 & 0 \\ 0 & 0 & 0 & 0 & 0 \end{array} \right] \xrightarrow {} \left\{ \begin{array}{l} r _ {0} + r _ {1} + r _ {5} = \mathbf {0} \\ r _ {0} + r _ {2} + r _ {6} = \mathbf {0} \\ r _ {3} + r _ {4} + r _ {7} = \mathbf {0} \end{array} \right\}
$$

In general, if $M _ { u }$ is $p \times h w ( \pmb { u } )$ , then there are $p - h w ( \pmb { u } )$ all-zero rows in the row echelon form given that $R a n k ( M _ { \mathbf { u } } ) = h w ( \mathbf { u } )$ . 

3. Find all the sets of linearly dependent rows. We do so by trying the combinations between the relations derived from all-zero rows obtained in the previous step, e.g., combine $r _ { 0 } + r _ { 1 } + r _ { 5 } = 0$ and $r _ { 0 } + r _ { 2 } + r _ { 6 } = 0$ will produce $r _ { 0 } + r _ { 1 } + r _ { 5 } + r _ { 0 } + r _ { 2 } + r _ { 6 } = \mathbf { 0 } \Rightarrow r _ { 1 } + r _ { 2 } + r _ { 5 } + r _ { 6 } = \mathbf { 0 }$ which means the rows $\{ r _ { 1 } , r _ { 2 } , r _ { 5 } , r _ { 6 } \}$ are linearly dependent. 

$$
\left\{ \begin{array}{c} r _ {0} + r _ {1} + r _ {5} = \mathbf {0} \\ r _ {0} + r _ {2} + r _ {6} = \mathbf {0} \\ r _ {3} + r _ {4} + r _ {7} = \mathbf {0} \\ r _ {1} + r _ {2} + r _ {5} + r _ {6} = \mathbf {0} \\ r _ {0} + r _ {1} + r _ {3} + r _ {4} + r _ {5} + r _ {7} = \mathbf {0} \\ r _ {0} + r _ {2} + r _ {3} + r _ {4} + r _ {6} + r _ {7} = \mathbf {0} \\ r _ {1} + r _ {2} + r _ {3} + r _ {4} + r _ {5} + r _ {6} + r _ {7} = \mathbf {0} \end{array} \right.
$$

4. For each set of linearly dependent rows, we derive a constraint on some bits of v enforcing any selected sub-matrix to be invertible, $e . g . , r _ { 0 } + r _ { 1 } + r _ { 5 } = \mathbf { 0 }$ means the rows $\{ r _ { 0 } , r _ { 1 } , r _ { 5 } \}$ are linearly dependent. In other words, these rows together must not be a part of any sub-matrix in order to have valid trails. Reflecting on ${ \pmb v } ,$ this means the bits $v _ { 0 } , v _ { 1 } , v _ { 5 }$ cannot be 1 at the same time. We can represent this relation as a linear constrain $v _ { 0 } + v _ { 1 } + v _ { 5 } \leq 2$ . The initial model for our toy linear layer includes: 

$$
\left\{ \begin{array}{c c} v _ {0} + v _ {1} + v _ {5} \leq 2 & \text {(C1)} \\ v _ {0} + v _ {2} + v _ {6} \leq 2 & \text {(C2)} \\ v _ {3} + v _ {4} + v _ {7} \leq 2 & \text {(C3)} \\ v _ {1} + v _ {2} + v _ {5} + v _ {6} \leq 3 & \text {(C4)} \\ v _ {0} + v _ {1} + v _ {3} + v _ {4} + v _ {5} + v _ {7} \leq 5 & \text {(C5)} \\ v _ {0} + v _ {2} + v _ {3} + v _ {4} + v _ {6} + v _ {7} \leq 5 & \text {(C6)} \\ v _ {1} + v _ {2} + v _ {3} + v _ {4} + v _ {5} + v _ {6} + v _ {7} \leq 6 & \text {(C7)} \\ v _ {0}, \dots , v _ {7} \text {are binary variables} \end{array} \right.
$$

5. Remove the redundancy constraints, e.g., the constraint C5 is redundant because if the constraints C1 and C3 are satisfied, then the constraint C5 is satisfied. Also, if the constraints C1 and C3 are not satisfied, then the constraint C5 is not satisfied. In contrast, if one of the constraints C1 and C3 is satisfied and the other is not satisfied, the solution will be rejected even though the constraint C5 is satisfied. 

We can automate this step by checking if a set of dependent rows (A) is a sub-set of another set of dependent rows (B), then the constraint on the set B is redundant. The model for our toy linear layer is then reduced to: 

$$
\left\{ \begin{array}{c} v _ {0} + v _ {1} + v _ {5} \leq 2 \\ v _ {0} + v _ {2} + v _ {6} \leq 2 \\ v _ {3} + v _ {4} + v _ {7} \leq 2 \\ v _ {1} + v _ {2} + v _ {5} + v _ {6} \leq 3 \\ v _ {0}, \ldots , v _ {7} \text { are binary variables } \end{array} \right.
$$

6. Finally, add a constraint to enforce that $h w ( \pmb { u } ) = h w ( \pmb { v } )$ . The model for our toy linear layer will be 

$$
\left\{ \begin{array}{c} v _ {0} + v _ {1} + v _ {5} \leq 2 \\ v _ {0} + v _ {2} + v _ {6} \leq 2 \\ v _ {3} + v _ {4} + v _ {7} \leq 2 \\ v _ {1} + v _ {2} + v _ {5} + v _ {6} \leq 3 \\ v _ {0} + v _ {1} + \dots + v _ {7} = 5 \\ v _ {0}, \ldots , v _ {7} \text {are binary variables} \end{array} \right.
$$

Number of Constraints. Although we cannot count exactly the number of the required constraints before performing the procedure, we can give the upper bound of the number based on Step 3 as follows: 

$$
\# \mathcal {L} \leq 1 + \sum_ {i = 1} ^ {p - h w (\boldsymbol {u})} \binom {p - h w (\boldsymbol {u})} {i} = 1 + 2 ^ {p - h w (\boldsymbol {u})} - 1 = 2 ^ {p - h w (\boldsymbol {u})}
$$

In the light of this upper bound, it is clear that the model is practically more applicable when $p - h w ( \pmb { u } )$ is relatively small which is usually the case for the linear layer at the first round when we search for a distinguisher that covers a large number of rounds where the Hamming weight of the input division property of the distinguisher (the number of active bits) is very close to the block size. 

# 4.2 Complete Model and Search Approach

In the previous section, we presented a model for the linear layer at the first round when its input division property is known before the modelling step. In this section, we propose a search approach allowing us to use that model even though the targeted distinguisher does not start from a linear layer. We also complete the model for the targeted distinguisher by showing how to handle the intermediate linear layers. 

Intermediate Linear Layers. We use the disjointed representation described in Sect. 3.1 to model the intermediate linear layers. When a candidate division tail is obtained by solving the complete model, we then extract the values of the input and the output division property of each matrix multiplication in the trail. After that, we check whether $M _ { v , u }$ is invertible or not for each matrix multiplications. If one of them is not invertible, we discard the trail by updating the model through adding a special craft constraint and resolving the updated model. 

Discarding Invalid Trails. Let $\left( u _ { 0 } , \ldots , u _ { n - 1 } \right)$ and $\left( v _ { 0 } , \ldots , v _ { n - 1 } \right)$ be the variables in the model representing the input and the output division property of a matrix multiplication where $M _ { v , u }$ is not invertible in the current solution of the model. 

Let $I _ { 0 } ^ { u } ( I _ { 1 } ^ { u } )$ be the indices of $\pmb { u } ^ { \prime } \mathrm { s }$ variables that equal to 0 (1) in the current solution. Similarly, let $I _ { 0 } ^ { v } ~ ( I _ { 1 } ^ { v } )$ be the indices of ${ \pmb v } ^ { \prime } \mathrm { s }$ variables that equal to 0 (1) in the current solution. We update the model based on the current solution by adding the following constraint 

$$
\sum_ {i \in I _ {0} ^ {u}} (u _ {i}) + \sum_ {i \in I _ {1} ^ {u}} (1 - u _ {i}) + \sum_ {i \in I _ {0} ^ {v}} (v _ {i}) + \sum_ {i \in I _ {1} ^ {v}} (1 - v _ {i}) \geq 1
$$

Therefore, when we attempt to resolve the updated model, the current solution, i.e., the invalid trial, will violate the new constraint and the solver will not consider it as a solution and try to obtain another solution. 

Implementation. Although the models for both the first linear layer with known input division property and the intermediate linear layers with the discarding approach above are applicable using MILP and SMT/SAT, the approach to discard invalid trails is more efficient using MILP solvers via the callback function and the concept of lazy constraints [9,11] without needing to resolve the model from scratch. 

Last Linear Layer. When the distinguisher ends with a linear layer, we can model it using the disjointed representation (like the intermediate linear layers) or we can efficiently model it using the model for XOR operation. Let $\left( u _ { 0 } , \ldots , u _ { n - 1 } \right)$ and $( v _ { 0 } , \ldots , v _ { n - 1 } )$ be the variables in the model which represent the input and the output division property of the matrix multiplication in the last linear layer. Suppose we check if there is a division trail from the input division property of the distinguisher to the unit vector $e _ { i } , i . e .$ , checking if the i-th bit of the output is balanced or not. Therefore, the variables that represent the output division property will be set to 

$$
\left\{ \begin{array}{l} v _ {i} = 1 \\ v _ {l} = 0, \qquad 0 \leq l \leq n - 1, l \neq i \end{array} \right.
$$

Consequently, during modelling, we focus on row $r _ { i } = M ( i , * )$ of the primitive matrix M and the constraints on the input division property of the matrix multiplication will be 

$$
\left\{ \begin{array}{l}\sum_{\substack{j = 0\\ M(i,j) = 1}}^{n - 1}u_{j} = 1\\ u_{j} = 0, \qquad 0\leq j\leq n - 1,M(i,j) = 0 \end{array} \right.
$$

After solving the model, if there is a division trail from the input division property of the distinguisher to the unit vector $e _ { i } ,$ we conclude that there are other division trails from the same input division property of the distinguisher to other unit vectors without creating/solving their corresponding models. The original division trial can be split into two sub-trails; from the input division property of the distinguisher to the input division property of the last linear layer ${ \pmb u } ,$ and from u to the unit vector $e _ { i }$ where $h w ( \pmb { u } ) = h w ( \pmb { v } ) = 1 , i . e .$ , only one variable from $\left( u _ { 0 } , \ldots , u _ { n - 1 } \right)$ is 1 and the other are 0. Suppose this variable is $u _ { j }$ . Therefore, the column matrix $M _ { u }$ can be created from a single column $c _ { j } = M ( * , j )$ . Based on Theorem 1, the division trail from the input division property of the distinguisher to the unit vector $e _ { l }$ , passing through u, exists for the l-th output bit if $M ( l , j ) = 1$ where $0 \leq l \leq n - 1$ . 

Search Approach. If the targeted distinguisher starts from a linear layer, the input division property of this linear layer is known and we can use the model described in Sect. 4.1. Hence, we create only one model for the distinguisher. Otherwise, we perform the following search approach: 

1. We firstly determine all the possible values of the input division property of the first linear layer by propagating the input division property of the distinguisher through other parts of the first round, which is usually a nonlinear layer of Sboxes. 

2. Then, we check the i-th output bit by creating a group of sub-models starting from the first linear layer with different input division property, thereby, we can employ the model described in Sect. 4.1 for the first linear layer in each sub-model. 

3. Finally, we solve the sub-models independently in parallel by dividing our computational power between them. If the valid division trail that ends at the unity vector $e _ { i }$ exists for a sub-model, we terminate the search process for the other sub-models. If it does not exist for all sub-models, then the i-th output bit is balanced. The last two steps are repeated for all output bits. 

Remark. Even though the model for the linear layer using the disjointed representation with discarding invalid trails approach is applicable to the first linear layer, we believe that modelling the first linear layer accurately from the beginning is important. Our reasoning for that is as follows. First, the Hamming weight of the input/output division property for the first linear layer is the highest compared to the successive linear layers, i.e., the number of its possible propagation is high and the chance to find invalid sub-trails will increase, which leads to the second reason. Since every sub-trail in early rounds is branched to many trails in the successive rounds, invalid sub-trails in the first round have a larger effect on expanding the search space, and hence increasing the time of solving the model. We verified our hypothesis experimentally by comparing the running time to find the 4-round key-dependent integral distinguish of AES reported in [10] using the same platform in the two cases; the case when the first linear layer is modelled accurately from the beginning and the other case when we model the first linear layer using the disjointed representation with discarding approach. In the first case, the solver found the distinguisher in around 50 min. In contrast, the solver did not finish in the second case even after running for more than a day. 

# 5 Applications of Our New Approach

In this section, we report our findings when applying our approach to Kuznyechik and Kalyna block ciphers and a variant of PHOTON permutations. We also have reproduced the results of the 4- and 5-round dependent-key integral distinguishers of AES reported in [10]. 

During our experiments, We use either Gurobi2 solver [9] or the CPLEX optimizer [11] to solve the models. Our source codes are available at https:// github.com/mhgharieb/MILP DivisionProprerty LinearLayer. 

We use the following notation to present the integral property of each byte in the plaintext and ciphertext: 

– : Each bit of the byte at the plaintext is fixed to constant. 

– : All bits of the byte at the plaintext are active. 

– : Each bit of the byte at the ciphertext is balanced (the XOR sum is zero). 

– : A byte at the ciphertext with unknown status (the XOR sum is unknown). 

When each bit of a byte has a different property, we use lowercase letters to present the property, i.e., c, a and b will represent a constant bit, an active bit, and a balanced bit, respectively. For example, caaaaaaa represents a byte where the most significant bit is constant and the other bits are active. 

In general during our experiments, when an R-round distinguisher is found, we follow two different paths in parallel as a next step; we examine whether (R + 1)-round exists or not, and we try to find another R-round distinguisher that needs a less number of active bits, i.e., less data complexity. 

# 5.1 Application to Kuznyechik

The Russian encryption standard—Kuznyechik [3], also known as GOST 34.12- 2015, is a 9-round SPN-based block cipher with a 128-bit block size and 256 bits of key. The encryption procedure is performed as follows. After loading a block of 128-bit plaintext to a 16-byte internal state ${ \pmb x } = ( x _ { 0 } , \dots , x _ { 1 5 } )$ where x0 is the least significant byte, the state is Xored with a whitening round key (XOR Layer (X)). Then, the state is updated 9 times using an identical round function denoted as $R = ( X \circ L \circ S )$ that consists of: 

– Non-linear Layer (S): Each byte of the state is mapped using 8-bit Sbox. 

– Linear Layer (L): The 16-byte state is multiplied by 16 16 MDS matrix over the field $\mathbb { F } _ { 2 ^ { 8 } }$ with the irreducible polynomial $X ^ { 8 } + X ^ { 7 } + X ^ { 6 } + X + 1$ . 

– XOR Layer (X): The state is Xored with the corresponding round key. 

In [1], Biryukov et al. studied Kuznyechik security against the multisetalgebraic cryptanalysis in which they reported the 3- and 4-round integral distinguisher based on their algebraic degree. 

3-Round Integral Distinguishers. Biryukov et al. reported that the 3-round has degree at most 116 [1]. Therefore the XOR sum over a set of plaintexts with dimension 117 will be zero, $i . e .$ , the 3-round integral distinguisher exists with the data complexity of $2 ^ { 1 1 7 }$ . However, we found several 3-round integral distinguishers with a much lower data complexity of $2 ^ { 5 6 }$ . One of these distinguishers is 

$$
\begin{array}{l} (\mathcal {C}, \mathcal {C}, \mathcal {C}, \mathcal {C}, \mathcal {C}, \mathcal {C}, \mathcal {C}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}) \\ \Downarrow 3 R \circ X \\ (\mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}) \\ \end{array}
$$

4-Round Integral Distinguishers. Biryukov et al. also reported a 4-round distinguisher with the data complexity of $2 ^ { 1 2 7 }$ depending on the 4-round has degree at most 126 [1]. We were able to find several 4-round integral distinguishers with data complexity of $2 ^ { 1 2 0 }$ (120 active bits). One of these distinguishers is 

$$
\begin{array}{l} (\mathcal {C}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}) \\ \Downarrow 4 R \circ X \\ (\mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}) \\ \end{array}
$$

Other Experiments. Biryukov et al. extended the 4-round key-independent integral distinguisher to a 5-round key-dependent one with the same data complexity by appending the linear layer (L) before the 4-round one. The new distinguisher depends on the least significant byte of the master key. We were able to verify the existence of this distinguisher using our model by setting one bit to a constant and the other bits to active as shown below. 

$$
\begin{array}{l} (\text { caaaaaaa }, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}, \mathcal {A}) \\ \Downarrow 4 R \circ X \circ L \\ (\mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}, \mathcal {B}) \\ \end{array}
$$

As a next step, we employ the search approach proposed in the previous section to check the existence of the 5-round key-independent distinguisher with a single bit constant and 127 bits active, and we confirmed that this distinguisher does not exist even with the use of the accurate propagation of the BDP. 

# 5.2 Application to Kalyna

The Ukrainian standard Kalyna [13], also known as DSTU 7624:2014, is a family of five SPN-based block ciphers denoted as Kalyna-l/k where $l , k \in$ 128, 256, 512 are the block size and the key size, respectively, such that $k = l$ or $k = 2 \times l$ . The number of rounds depends on the key size. 

We targeted the two members with the block size of 128 bits, Kalyna-128. The encryption procedure is performed as follows. The 16 bytes of the plaintext block ${ \pmb x } = ( x _ { 0 } , \dots , x _ { 1 5 } )$ where $x _ { 0 }$ is the least significant byte, is loaded to the $8 \times 2$ 16-byte state matrix in column-wise order. After that, pre-whitening round key is added to each column independently using addition modulo $2 ^ { 6 4 }$ . We denote this operation as $\left( \boxplus _ { 6 4 } \right)$ . Then, The following round function denoted as $R =$ $( X \circ L \circ S R \circ S )$ is iterated 10 or 14 times depending on the key size: 

– Non-linear Layer (S): 4 different 8-bit Sboxes $\pi _ { s } , s \in \{ 0 , 1 , 2 , 3 \}$ are used to map the bytes of the state matrix where the i-th byte $( x _ { i } )$ is substituted by $\pi _ { i \mod 4 } ( x _ { i } )$ . 

– ShiftRows (SR): The bytes of each row in the state matrix are cyclically shifted to right by $\left\lfloor { \frac { i } { 4 } } \right\rfloor$ where $i , 0 \le i \le 7$ is the row index. 

– Linear Layer (L): Each 8-byte column of the state matrix is independently multiplied by $8 \times 8$ MDS matrix over the field $\mathbb { F } _ { 2 ^ { 8 } }$ with the irreducible polynomial $X ^ { 8 } + X ^ { 4 } + X ^ { 3 } + X ^ { 2 } + 1$ . 

– XOR Layer (X): the state is Xored with the corresponding round key. 

In the last round, the XOR Layer (X) is replaced by a post-whitening modular key addition modulo $2 ^ { 6 4 }$ . 

4-Round Integral Distinguishers Without Pre-whitening. During our experiments, we found two 4-round integral distinguisher starting after the prewhitening step with 8 active bytes as depicted below. The correctness of these distinguishers can be easily verified by propagating the integral properties though the equivalent structure of the round function. Given that, each 8-bit Sbox is reused every 4 bytes and the first (second) 4 rows of the state matrix is shifted by the same step, the state matrix can be reconstructed as $2 \times 2$ matrix such that each 4 successive bytes are concatenated in a 32-bit word and the 4 different 8-bit Sboxes build a 32-bit super Sbox. Therefore, when the diagonal (anti-diagonal) words of the new state matrix are active, i.e., take all possible values from $\mathbb { F } _ { 2 ^ { 3 2 } } ^ { 2 }$ , the output after 4-rounds will be balanced similar to the 4-round integral distinguisher of AES [12]. 

$$
\left[ \begin{array}{c c} \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \end{array} \right] \mathrm{OR} \left[ \begin{array}{c c} \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \end{array} \right] \overset {\underset {\longrightarrow} {4 R}} {\Longrightarrow} \left[ \begin{array}{c c} \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \end{array} \right] \overset {\underset {\longrightarrow} {A p p e n d i n g Ⅲ ⑥ 4}} {\Longrightarrow} \left[ \begin{array}{c c} \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {C} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} \end{array} \right] \mathrm{OR} \left[ \begin{array}{c c} \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} \\ \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} \end{array} \right] \overset {\underset {\longrightarrow 4 R ⓞ ⑥ 4}} {\Longrightarrow} \left[ \begin{array}{c c} \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ \mathcal {B} & \mathcal {B} \\ {\bf B}, {\bf B}, {\bf C}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf C}, {\bf D}, {\bf A}, {\bf B}, {\bf C}, {\bf D}, {\bf A}, {\bf B}. \\ [ 1, 2 ] = [ 1, 2 ] ^ {- 1 / 2}.
$$

4-Round Integral Distinguishers with Pre-whitening. We were able to extend each of the previous 4-round distinguishers to cover the pre-whitening operation. The new distinguishers need 12 active bytes as depicted above. In the following, we illustrate the way we use to select a set of plaintexts so that it satisfies the input division property of the 4-round distinguisher after applying the pre-whitening operation. 

Since the pre-whitening operation is performed per column, we focus on each column independently. Suppose X, Y , and K denote a 64-bit word of the input, the output and the whitening key, respectively, such that $Y = X \boxplus _ { 6 4 } K$ . Each 64-bit word can be considered as the concatenation of two 32-bit words, $i . e .$ , $X = X _ { l } | | X _ { r } , Y = Y _ { l } | | Y _ { r }$ , and $K = K _ { l } | | K _ { r }$ . Therefore, $Y _ { r } = X _ { r } \boxplus _ { 3 2 } K _ { r }$ and $Y _ { l } = X _ { l } \boxplus _ { 3 2 } K _ { l } \boxplus _ { 3 2 } C$ where $\boxplus _ { 3 2 }$ denotes the addition modulo $2 ^ { 3 2 }$ and C is the carry from the first addition part. 

Consequently, a set of plaintexts such that $X _ { r }$ is fixed to constant and the 4 bytes of $X _ { l }$ takes all the possible values from $\mathbb { F } _ { 2 ^ { 8 } } ^ { 4 }$ , will give an output set such that $Y _ { r }$ will be constant and the 4 bytes of $Y _ { r }$ will take all the possible values from $\mathbb { F } _ { 2 ^ { 8 } } ^ { 4 }$ . This is because the whitening key is constant and the carry will be fixed over all the set’s elements based on the previous two questions. As the result, we can easily satisfy one of the two column in the 4-round distinguishers. 

The same method cannot be applied to the other column because if $X _ { r }$ takes all the possible values, $Y _ { r }$ will take all the possible values, but, the value of the carry will change depending on the value of the whitening key. Hence, we cannot adapt the values of $X _ { l }$ to enforce $Y _ { l }$ to be fixed over the set. To overcome this problem, we construct a set of plaintexts such that the 8 bytes of X take all the possible values from $\mathbb { F } _ { 2 ^ { 8 } } ^ { 8 }$ , hence, the 8 bytes of $Y$ will take all the possible values from $\mathbb { F } _ { 2 ^ { 8 } } ^ { 8 }$ . As the result, the output set $Y$ can be considered as $2 ^ { \bar { 3 } 2 }$ sub-sets in which each sub-set satisfies the input division property of the other column of the 4-round distinguisher. Combining these two approaches, the 4-round distinguishers with the pre-whitening need 12 active bytes. 

Using the BDP, we are able to verify the existence of these distinguishers with the help of the propagation model of the BDP through modular addition with a constant proposed in [4]. Additionally, we have tried to reduce the number of active bits by iterating over the active bits one-by-one and set it to constant then check if the distinguisher still exists. Unfortunately, the distinguisher does not exist. 

Other Experiments. During our experiments, we build a 4-round keydependent distinguisher using 62 active bits. The new distinguisher depends on the 32 least significant bits of the pre-whitening key. The distinguisher starts at the linear layer of the first round with the input division property. For more details, we refer the redear to the long version of this paper [7]. 

# 5.3 Application to PHOTON

PHOTON [8] is a family of lightweight hash functions proposed by Guo et al. at CRYPTO 2011 and it has been standardized in ISO/IEC 29192-5:2016. PHO-TON has 5 variants with 5 internal unkeyed permutations denoted as $P _ { t }$ where $t \in \{ 1 0 0 \}$ , 144, 196, 256, 288 is the internal state size. We target here the internal permutation $P _ { 2 8 8 }$ . The structure of the internal permutation follows the structure of AES where the internal state is represented as a $d \times d$ square matrix of cells. Thus, the internal state of $P _ { 2 8 8 }$ is a $6 \times 6$ matrix of bytes. Its round function consists of: 

– AddConstants (X): Each byte of the 1st column of the state matrix is Xored with a round-dependent constant. 

– SubCells (S): Each byte $( x _ { i } )$ of the state is substituted by $S b o x ( x _ { i } )$ where Sbox is the 8-bit Sbox of AES. 

– ShiftRows (SR): The bytes of each row in state are cyclically shifted to left by i where $i \in { 0 } \le i \le 5$ is the row index. 

– MixColumnsSerial (L): Each column of the state is independently multiplied by $6 \times 6$ MDS matrix over $\mathbb { F } _ { 2 ^ { 8 } }$ with the irreducible polynomial $X ^ { 8 } + X ^ { 4 } +$ $X ^ { 3 } + X + 1$ . 

3- and 4-Round Integral Distinguishers. Since the permutation is followed the AES structure, there are 3- and 4-round distinguishers that exploit the structure itself and independent on the used Sboxes and the MDS matrix. In particular, when the state matrix has a single byte active and the other bytes are constant (the data complexity is $2 ^ { 8 } )$ , each output bit after 3 rounds will have zero-sum (balanced). Also, there is a 4-round distinguisher when all diagonal’s bytes of the state matrix are active (the data complexity is $2 ^ { 4 8 } )$ . In [15], Sun et al. verified the existence of these 3- and 4-round distinguishers using the MILP models for the propagation of the BDP. They have modelled the linear layer using the disjointed representation. 

New 4-Round Integral Distinguisher. At Crypto 2016, Sun et al. exploited a specific property of the matrix used in AES to introduce the first 5-round key-dependent integral distinguisher [14]. This property is that each column of the matrix has two equal elements. We employ a similar property to reduce the date complexity of the 4-round distinguisher of $P _ { 2 8 8 }$ and build a new 5-round one. 

Suppose $M _ { P }$ and $M _ { P } ^ { - 1 }$ denote the matrix and its inverse that are used in $P _ { 2 8 8 }$ where 

$$
M _ {P} = \left[ \begin{array}{l l l l l l} 0 2 & 0 3 & 0 1 & 0 2 & 0 1 & 0 4 \\ 0 8 & 0 \mathrm{e} & 0 7 & 0 9 & 0 6 & 1 1 \\ 2 2 & 3 \mathrm{b} & 1 \mathrm{f} & 2 5 & 1 8 & 4 2 \\ 8 4 & \mathrm{e} 4 & 7 9 & 9 \mathrm{b} & 6 7 & 0 \mathrm{b} \\ 1 6 & 9 9 & \mathrm{ef} & 6 \mathrm{f} & 9 0 & 4 \mathrm{b} \\ 9 6 & \mathrm{cb} & \mathrm{d} 2 & 7 9 & 2 4 & \mathrm{a} 7 \end{array} \right], M _ {P} ^ {- 1} = \left[ \begin{array}{l l l l l l} 1 5 & 5 0 & \mathrm{eb} & 6 2 & 7 9 & 9 9 \\ 2 9 & \mathrm{a} 5 & \mathrm{c} 9 & \mathrm{c} 2 & \mathrm{fb} & 2 \mathrm{b} \\ 5 6 & 5 4 & 8 \mathrm{e} & 9 \mathrm{f} & \mathrm{e} 9 & 5 7 \\ \mathrm{ae} & \mathrm{af} & 0 3 & 2 0 & \mathrm{c} 8 & \mathrm{ae} \\ 4 7 & 4 7 & 0 1 & 4 4 & 8 \mathrm{e} & 4 6 \\ 8 \mathrm{c} & 8 \mathrm{d} & 0 1 & 8 \mathrm{d} & 0 2 & 8 \mathrm{d} \end{array} \right]
$$

Suppose ${ \pmb x } = ( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } , x _ { 4 } , x _ { 4 } ) ^ { T }$ and $\pmb { y } = ( y _ { 0 } , y _ { 1 } , y _ { 2 } , y _ { 3 } , y _ { 4 } , y _ { 5 } ) ^ { T }$ be the input and the output vectors to the matrix $M _ { P }$ such that $\pmb { y } = M _ { P } \times \pmb { x }$ . Suppose x take $2 ^ { 5 \times 8 = 4 0 }$ values where each of $x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 }$ and $x _ { 4 }$ take all the possible values from $\mathbb { F } _ { 2 ^ { 8 } }$ . Therefore, y will take $2 ^ { 4 0 }$ values. Also, $\pmb { x } = M _ { P } ^ { - 1 } \times \pmb { y }$ can be 

expressed as shown below 

$$
\left[ \begin{array}{c} x _ {0} \\ x _ {1} \\ x _ {2} \\ x _ {3} \\ x _ {4} \\ x _ {4} \end{array} \right] = \left[ \begin{array}{c c c c c c} 1 5 & 5 0 & \mathrm{eb} & 6 2 & 7 9 & 9 9 \\ 2 9 & \mathrm{a5} & \mathrm{c9} & \mathrm{c2} & \mathrm{fb} & 2 \mathrm{b} \\ 5 6 & 5 4 & 8 \mathrm{e} & 9 \mathrm{f} & \mathrm{e9} & 5 7 \\ \mathrm{ae} & \mathrm{af} & 0 3 & 2 0 & \mathrm{c8} & \mathrm{ae} \\ 4 7 & 4 7 & 0 1 & 4 4 & 8 \mathrm{e} & 4 6 \\ 8 \mathrm{c} & 8 \mathrm{d} & 0 1 & 8 \mathrm{d} & 0 2 & 8 \mathrm{d} \end{array} \right] \left[ \begin{array}{c} y _ {0} \\ y _ {1} \\ y _ {2} \\ y _ {3} \\ y _ {4} \\ y _ {5} \end{array} \right]
$$

Hence, we can express $x _ { 4 }$ as follows in Eqs. (1) and (2). 

$$
x _ {4} = 4 7 \cdot y _ {0} \oplus 4 7 \cdot y _ {1} \oplus 0 1 \cdot y _ {2} \oplus 4 4 \cdot y _ {3} \oplus 8 e \cdot y _ {4} \oplus 4 6 \cdot y _ {5} \tag {1}
$$

$$
x _ {4} = 8 \mathrm{c} \cdot y _ {0} \oplus 8 \mathrm{d} \cdot y _ {1} \oplus 0 1 \cdot y _ {2} \oplus 8 \mathrm{d} \cdot y _ {3} \oplus 0 2 \cdot y _ {4} \oplus 8 \mathrm{d} \cdot y _ {5} \tag {2}
$$

$$
0 0 = \mathbf {c b} \cdot y _ {0} \oplus \mathbf {c a} \cdot y _ {1} \oplus 0 0 \cdot y _ {2} \oplus \mathbf {c 9} \cdot y _ {3} \oplus 8 \mathbf {c} \cdot y _ {4} \oplus \mathbf {c b} \cdot y _ {5} \tag {3}
$$

From (1) and (2), we can derive the Eq. (3) which implies that $\{ y _ { 0 } , y _ { 1 } , y _ { 3 }$ $y _ { 4 } , y _ { 5 } \}$ are linearly dependent, i.e., they can take at most ${ \bar { 2 } } ^ { 4 \times 8 = 3 2 }$ values. Since y takes $2 ^ { 4 0 }$ values, y2 must take $2 ^ { 8 }$ values, i.e., $y _ { 2 }$ is an active bye and takes its all possible values ( ). 

Constructing 4-Round Integral Distinguisher. We construct a set of $2 ^ { 4 0 }$ chosen plaintexts such that the state matrix is as follows. The first 4 elements of the diagonal are active, the last two elements of the diagonal are equal and active (denoted as $)$ , and the other elements of the state matrix are fixed to constant as shown below. After applying the three operations: AddConstants (X), SubCells (S), and ShiftRows (SR), the first column of the state matrix will be in the form of the vector x. Therefore, the output set, after applying the MixColumnsSerial (L) operation (a full round from the input set), can be divided into $2 ^ { 3 2 }$ subset so that each has one active byte and the other are constant. Consequently, after another 3 rounds, each bit of the output will have zero-sum as mentioned previously in the 3-round distinguisher section. 

$$
\left[ \begin{array}{c c c c c} \mathcal {A}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C} \\ \mathcal {C}   \mathcal {A}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C} \\ \mathcal {C}   \mathcal {C}   \mathcal {A}   \mathcal {C}   \mathcal {C}   \mathcal {C} \\ \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {A}   \mathcal {C}   \mathcal {C} \\ \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \bar {\mathcal {A}}   \mathcal {C} \\ \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \bar {\mathcal {A}} \end{array} \right] \xrightarrow {S R \circ S \circ X} \left[ \begin{array}{c c c c c} \mathcal {A}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C} \\ \mathcal {A}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C} \\ \mathcal {A}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C}   \mathcal {C} \\ \mathcal {A}   \mathcal {C}   \texttt {C C C C C C} \\ \bar {\mathcal {A}}   \texttt {C C C C C C} \\ \bar {\mathcal {A}}   \texttt {C C C C C C} \end{array} \right] \xrightarrow [ ]{\underline {{L}}} 2 ^ {3 2} \times \left\{\left[ \begin{array}{c c c c c} \mathcal {C} & \mathcal {C C C C C C} \\ \mathcal {C C C C C C C} \\ \mathcal {A C C C C C C} \\ \mathcal {C C C C C C C} \\ \mathcal {C C C C C C C} \\ \mathcal {C C C C C C C} \\ \mathcal {C C C C C C C} \end{array} \right] \xrightarrow [ ]{\underline {{3 R}}} \left[ \begin{array}{c c c c c} B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \\ B & B & B & B & B \end{array} \right] \right\}
$$

MILP for the New 4-Round Distinguisher. Our model can be started at the MixColumnsSerial (L) operation of the first round, therefore, we can use the accurate model for the propagation of the BDP described in Sect. 4.1. The first column of the state matrix (in the form of x) will be multiplied by $M _ { P }$ . Since the last two element of the vector x are equal, we can express the multiplication operation $\pmb { y } = M _ { P } \times \pmb { x }$ as $\pmb { y } = \hat { M } _ { P } \times \hat { \pmb { x } }$ where ${ \pmb { \hat { x } } } = ( x _ { 0 } , x _ { 1 } , x _ { 2 } , x _ { 3 } , x _ { 4 } ) ^ { T }$ and $\hat { M } _ { P }$ is as follows. 

$$
\left[ \begin{array}{l} y _ {0} \\ y _ {1} \\ y _ {2} \\ y _ {3} \\ y _ {4} \\ y _ {5} \end{array} \right] = \left[ \begin{array}{l l l l l} 0 2 & 0 3 & 0 1 & 0 2 & 0 5 \\ 0 8 & 0 \mathrm{e} & 0 7 & 0 9 & 1 7 \\ 2 2 & 3 \mathrm{b} & 1 \mathrm{f} & 2 5 & 5 \mathrm{a} \\ 8 4 & \mathrm{e} 4 & 7 9 & 9 \mathrm{b} & 6 \mathrm{c} \\ 1 6 & 9 9 & \mathrm{ef} & 6 \mathrm{f} & \mathrm{db} \\ 9 6 & \mathrm{cb} & \mathrm{d} 2 & 7 9 & 8 3 \end{array} \right] \left[ \begin{array}{l} x _ {0} \\ x _ {1} \\ x _ {2} \\ x _ {3} \\ x _ {4} \end{array} \right] \triangleq \hat {M} _ {P} \left[ \begin{array}{l} x _ {0} \\ x _ {1} \\ x _ {2} \\ x _ {3} \\ x _ {4} \end{array} \right]
$$

Consequently, we use the primitive matrix of $\hat { M } _ { P }$ for the first column and the primitive matrix of $M _ { P }$ for other columns. Regarding the intermediate linear layers, we use the disjointed representation with discarding the invalid trails approach presented at Sect. 4.2. The result of solving the model is that a valid division trail that ends at a unit vector does not exist for any output bits, i.e., each output bit after 4 rounds will have zero-sum. It should be mentioned that the model of the first linear layer using the disjointed representation and not discarding the invalid trails leads some bits to be imbalanced. 

5-Round Integral Distinguisher. Similar to the new 4-round one, we employed the same property of the matrix $M _ { P }$ to build the 5-round distinguisher. We firstly construct a set of $2 ^ { 2 8 0 }$ chosen plaintexts where the last two elements of the diagonal are active and equal (denoted as $\bar { \mathcal { A } } )$ , and the other elements of the state matrix are active. This set can be divided, after the first round, into $2 ^ { 2 3 2 }$ sub-sets such that every sub-set has 6 bytes active at specific positions as shown below. Therefore, each sub-set can be considered as an input to 4-round distinguisher that exploit the structure of the round function. 

$$
\left[ \begin{array}{c c c c c c} \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} & \bar {\mathcal {A}} & \bar {\mathcal {A}} & \bar {\mathcal {A}} \\ \mathcal {A} & \mathcal {A} & \bar {\mathcal {A}} & \bar {\mathcal {A}} & \bar {\mathcal {A}} \end{array} \right] \xrightarrow {S R \circ S \circ X} \left[ \begin{array}{c c c c c c} \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} \\ \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} \\ \bar {\mathcal {A}} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \mathcal {A} & \bar {\mathcal {A}} \\ \bar {\mathcal {A}} & \bar {\mathcal {A}} & \bar {\mathcal {A}} & \bar {\mathcal {A}} & \bar {\mathcal {A}} & \bar {\mathcal {A}} \end{array} \right] \xrightarrow [ ]{\underline {{L}}} 2 ^ {2 3 2}   \times   \left\{\left[ \begin{array}{c c c c c c} \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} \\ \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} \\ \mathcal {A} & \mathcal {C} & \mathcal {C} & \mathcal {C} & \mathcal {C} & \bar {\mathcal {C}} \\ \mathcal {C} & \mathcal {C} & \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} \\ \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} \\ \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}} \\ \bar {\mathcal {C}} & \bar {\mathcal {C}} & \bar {\mathcal {C}}   & \bar {\mathcal {C}}   & \bar {\mathcal {C}}   \\ \end{array} \right] \xrightarrow [ ]{\underline {{4 R}}}   \left[ \begin{array}{c c c c c c} B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B   B    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    b    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    h    m    m     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     n     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     m     k      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l      l       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       p       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       P       M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M             M                     N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N           N---------------N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          N          O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O         O        C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C            C
$$

MILP for the 5-Round Distinguisher. We have followed the same steps as modelling the 4-round distinguisher to model the 5-round one, where we use the primitive matrix of $\hat { M } _ { P }$ for the first column multiplication in the first round at which the model starts and the primitive matrix of $M _ { P }$ for the other columns. The result of solving the model indicates that each output bit after 5 rounds is balanced. 

Other Experiments. We have employed our search approach (Sect. 4.2) to build a regular 5-round distinguisher that does not exploit the previous property of the matrix. We verified that this kind of distinguisher does not exist even when the number of active bits are 287 bits. Also, we have tried to reduce the number of active bits in both the regular and the new 4-round distinguisher by setting one of the active bits to constant and resolving the model. We verified that a distinguisher using less number of active bits does not exist. 

# 6 Conclusions

In this paper, we proposed a new MILP model for the propagation of the BDP through non-bit-permutation linear layers. To the best of our knowledge, this model is the most efficient one for large linear layers. With the help of our model, we improved the previous 3- and 4-round integral distinguishers of Kuznyechik and the 4-round one of PHOTON’s internal permutation $( P _ { 2 8 8 } )$ . We also found, for the first time, two 4-round integral distinguishers for Kalyna block cipher and a 5-round integral distinguisher for PHOTON’s internal permutation (P288). 

# References



1. Biryukov, A., Khovratovich, D., Perrin, L.: Multiset-algebraic cryptanalysis of reduced Kuznyechik, Khazad, and secret SPNs. IACR Trans. Symmetric Cryptol. 2016(2), 226–247 (2017) 





2. Bogdanov, A., et al.: PRESENT: an ultra-lightweight block cipher. In: Paillier, P., Verbauwhede, I. (eds.) CHES 2007. LNCS, vol. 4727, pp. 450–466. Springer, Heidelberg (2007). https://doi.org/10.1007/978-3-540-74735-2 31 





3. Dolmatov, V.: GOST R 34.12-2015: Block Cipher “Kuznyechik”. RFC 7801, RFC Editor, March 2016. https://tools.ietf.org/html/rfc7801 





4. ElSheikh, M., Tolba, M., Youssef, A.M.: Integral attacks on round-reduced Bel-T-256. In: Cid, C., Jacobson, M., Jr. (eds.) SAC 2018. LNCS, vol. 11349, pp. 73–91. Springer, Cham (2019). https://doi.org/10.1007/978-3-030-10970-7 4 





5. ElSheikh, M., Youssef, A.M.: A cautionary note on the use of Gurobi for cryptanalysis. Cryptology ePrint Archive, Report 2020/1112 (2020). https://eprint.iacr. org/2020/1112 





6. ElSheikh, M., Youssef, A.M.: Integral cryptanalysis of reduced-round tweakable TWINE. In: Krenn, S., Shulman, H., Vaudenay, S. (eds.) CANS 2020. LNCS, vol. 12579, pp. 485–504. Springer, Cham (2020). https://doi.org/10.1007/978-3-030- 65411-5 24 





7. ElSheikh, M., Youssef, A.M.: On MILP-based automatic search for bit-based division property for ciphers with (large) linear layers. Cryptology ePrint Archive, Report 2021/643 (2021). https://eprint.iacr.org/2021/643 





8. Guo, J., Peyrin, T., Poschmann, A.: The PHOTON family of lightweight hash functions. In: Rogaway, P. (ed.) CRYPTO 2011. LNCS, vol. 6841, pp. 222–239. Springer, Heidelberg (2011). https://doi.org/10.1007/978-3-642-22792-9 13 





9. Gurobi Optimization, LLC: Gurobi Optimizer Reference Manual (2020). http:// www.gurobi.com 





10. Hu, K., Wang, Q., Wang, M.: Finding bit-based division property for ciphers with complex linear layers. IACR Trans. Symmetric Cryptol. 2020(1), 396– 424 (2020). https://doi.org/10.13154/tosc.v2020.i1.396-424. https://tosc.iacr.org/ index.php/ToSC/article/view/8570 





11. IBM: IBM ILOG CPLEX 12.10 User’s Manual (2020). https://www.ibm.com/ support/knowledgecenter/SSSA5P 12.10.0/COS KC home.html 





12. Knudsen, L., Wagner, D.: Integral cryptanalysis. In: Daemen, J., Rijmen, V. (eds.) FSE 2002. LNCS, vol. 2365, pp. 112–127. Springer, Heidelberg (2002). https://doi. org/10.1007/3-540-45661-9 9 





13. Oliynykov, R., et al.: A new encryption standard of Ukraine: the Kalyna block cipher. Cryptology ePrint Archive, Report 2015/650 (2015). https://eprint.iacr. org/2015/650 





14. Sun, B., Liu, M., Guo, J., Qu, L., Rijmen, V.: New insights on AES-like SPN ciphers. In: Robshaw, M., Katz, J. (eds.) CRYPTO 2016. LNCS, vol. 9814, pp. 605–624. Springer, Heidelberg (2016). https://doi.org/10.1007/978-3-662-53018- 4 22 





15. Sun, L., Wang, W., Wang, M.Q.: MILP-aided bit-based division property for primitives with non-bit-permutation linear layers. IET Inf. Secur. 14, 12–20 (2020). https://digital-library.theiet.org/content/journals/10.1049/iet-ifs.2018.5283 





16. Todo, Y.: Structural evaluation by generalized integral property. In: Oswald, E., Fischlin, M. (eds.) EUROCRYPT 2015. LNCS, vol. 9056, pp. 287–314. Springer, Heidelberg (2015). https://doi.org/10.1007/978-3-662-46800-5 12 





17. Todo, Y.: Integral cryptanalysis on full MISTY1. J. Cryptol. 30(3), 920–959 (2016). https://doi.org/10.1007/s00145-016-9240-x 





18. Todo, Y., Morii, M.: Bit-based division property and application to Simon family. In: Peyrin, T. (ed.) FSE 2016. LNCS, vol. 9783, pp. 357–377. Springer, Heidelberg (2016). https://doi.org/10.1007/978-3-662-52993-5 18 





19. Xiang, Z., Zhang, W., Bao, Z., Lin, D.: Applying MILP method to searching integral distinguishers based on division property for 6 lightweight block ciphers. In: Cheon, J.H., Takagi, T. (eds.) ASIACRYPT 2016. LNCS, vol. 10031, pp. 648–678. Springer, Heidelberg (2016). https://doi.org/10.1007/978-3-662-53887-6 24 





20. Zhang, W., Rijmen, V.: Division cryptanalysis of block ciphers with a binary diffusion layer. IET Inf. Secur. 13, 87–95 (2019). https://digital-library.theiet.org/ content/journals/10.1049/iet-ifs.2018.5151 

