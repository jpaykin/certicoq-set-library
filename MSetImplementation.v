Require Import MSets.MSetRBT.

From CertiCoq.Plugin Require Import CertiCoq.

Require Import Coq.Structures.Orders.
From Coq.Numbers.Cyclic.Int63 Require Import PrimInt63.
Require Import CertiCoq.Plugin.PrimInt63.


(* API for MSets, which can be instantiated with any MSet implementation. *)
Record MSet_struct A S :=
    MSet_Struct {
        mset_empty : S;
        mset_mem : A -> S -> bool;
        mset_add : A -> S -> S;
        mset_cardinal : S -> int;
        mset_elements : S -> list A
    }.

(* Skeleton definition of primitive ints as an ordered type *)
Module int_as_OT <: UsualOrderedType.
    Definition t := int.
    Definition eq := @eq int.
    
    Definition eq_refl := @eq_refl t.
    Definition eq_sym := @eq_sym t.
    Definition eq_trans := @eq_trans t.
    Definition eq_equiv : Equivalence eq.
    Proof.
        constructor. exact eq_refl. exact eq_sym. exact eq_trans.
    Defined.

    Definition lt := fun z1 z2 => compare z1 z2 = Lt.

    Lemma lt_trans : forall x y z : t, lt x y -> lt y z -> lt x z.
    Admitted.

    Lemma lt_not_eq : forall x y : t, lt x y -> ~ eq x y.
    Admitted.

    Lemma int_eqb_eq : forall (x y : int),
        eqb x y = true <-> x = y.
    Admitted.

    Lemma leb_implies_gtb : forall (x y : int),
        ltb x y = false ->
        eqb x y = false ->
        ltb y x = true.
    Admitted.

    Lemma lt_strorder : StrictOrder lt.
    Admitted. 
    Lemma lt_compat : Proper (Logic.eq ==> Logic.eq ==> iff) lt.
    Admitted.

    Definition compare : int -> int -> comparison := compare.

    Lemma compare_spec : forall x y : t,
            CompareSpec (x = y) (lt x y) (lt y x) (compare x y).
    Admitted.

    Lemma compare_eq : forall x y, compare x y = Eq -> x = y.
    Admitted.
    Lemma compare_lt : forall x y, compare x y = Lt -> lt x y.
    Admitted.
    Lemma compare_gt : forall x y, compare x y = Gt <-> compare y x = Lt.
    Admitted.

    Definition eq_dec : forall x y : t, { eq x y } + { ~ eq x y }.
    Proof.
        intros x y.
        destruct (compare x y) eqn:Hxy.
        * left. apply compare_eq. exact Hxy.
        * right. apply lt_not_eq. exact Hxy.
        * right. apply compare_gt in Hxy. apply lt_not_eq in Hxy.
          intros Heq. apply Hxy. symmetry. exact Heq.
    Defined.
    
End int_as_OT.


Fixpoint nat_to_int (x : nat) : int :=
    match x with
    | 0 => 0
    | S x' => add 1 (nat_to_int x')
    end.


(* Instantiate red black trees (RBT) MSets with integers. *)
Module RBT := Coq.MSets.MSetRBT.Make int_as_OT.


Definition RBT_foldl {B} (f : B -> int -> B) (b : B) (s : RBT.t) : B :=
    Coq.Lists.List.fold_left f (RBT.elements s) b.

(* Implement RBT_size so it doesn't recurse over primitive nats directly,
   avoiding stack overflow *)
Definition RBT_size (s : RBT.t) : int :=
    let f := fun (acc : int) (_ : int) => add 1 acc
    in RBT_foldl f 0%int63 s.
(*
Definition RBT_size (s : RBT.t) : int :=
    let f := fun  (_ : int) (acc : int) => add 1 acc
    in RBT.fold f s 0%int63.
*)


Definition RBT_MSet_struct : MSet_struct int RBT.t :=
    {|
        mset_empty := RBT.empty;
        mset_mem := RBT.mem;
        mset_add := RBT.add;
        (*mset_cardinal := fun x => nat_to_int (RBT.cardinal x);*)
        mset_cardinal := RBT_size;
        mset_elements := RBT.elements;
    |}.

(* Finite Maps *)

Require Import Coq.FSets.FMapWeakList.
Record FMap_struct K V M :=
    {
        fmap_empty : M;
        fmap_is_empty : M -> bool;
        fmap_add : K -> V -> M -> M;
        fmap_find : K -> M -> option V;
        fmap_remove : K -> M -> M;
        fmap_mem : K -> M -> bool;
        fmap_equal : (V -> V -> bool) -> M -> M -> bool;
        fmap_elements : M -> list (K * V);
        fmap_size : M -> int;
    }.

Module AVLMap := Coq.FSets.FMapWeakList.Make(int_as_OT).
Definition anytype := bool.
Definition AVLMap_Instance : FMap_struct int anytype (AVLMap.t anytype) :=
    {|
        fmap_empty := @AVLMap.empty anytype;
        fmap_is_empty := @AVLMap.is_empty anytype : AVLMap.t anytype -> bool;
        fmap_add := @AVLMap.add anytype;
        fmap_find := @AVLMap.find anytype;
        fmap_remove := @AVLMap.remove anytype;
        fmap_mem := @AVLMap.mem anytype;
        fmap_equal := @AVLMap.equal anytype;
        fmap_elements := @AVLMap.elements anytype;
        fmap_size := fun x => nat_to_int (AVLMap.cardinal x);
    |}.


(* Export *)

Record CertiCoqLib := {
    CertiCoqSet : MSet_struct int RBT.t;
    CertiCoqMap : FMap_struct int anytype (AVLMap.t anytype)
    }.
Definition CertiCoqLibImpl : CertiCoqLib := {|
    CertiCoqSet := RBT_MSet_struct;
    CertiCoqMap := AVLMap_Instance
|}.

CertiCoq Generate Glue -file "glue" [bool, nat, option, list].
CertiCoq Compile -file "RBT" CertiCoqLibImpl
    Extract Constants []
    Include ["stdbool.h" as library].