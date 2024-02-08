#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

typedef enum {
    Abs,
    Atomic,
    Bool,
    And,
    Or,
    Not,
    Imply,
    A,
    E,
    X,
    F,
    G,
    U,
    R
} OpCode;

class Formula {
   public:
    OpCode opcode;
    std::vector<std::shared_ptr<Formula>> subformulas;
    std::vector<std::string> symbols;

    Formula(OpCode opcode, std::vector<std::shared_ptr<Formula>> subformulas,
            std::vector<std::string> symbols)
        : opcode(opcode), subformulas(subformulas), symbols(symbols) {}

    virtual std::string str() const = 0;
    virtual std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const = 0;
    virtual std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const = 0;
    virtual bool is_a_state_formula() const = 0;
};

class PathQuantifier : public Formula {
   public:
    using Formula::Formula;
    std::string str() const override {
        return symbols[0] + "(" + subformulas[0]->str() + ")";
    }

    bool is_a_state_formula() const override { return true; }
};

class TemporalOperator : public Formula {
   public:
    using Formula::Formula;
    std::string str() const override {
        if (subformulas.size() == 1) {
            return symbols[0] + "(" + subformulas[0]->str() + ")";
        } else {
            return "(" + subformulas[0]->str() + " " + symbols[0] + " " +
                   subformulas[1]->str() + ")";
        }
    }
    bool is_a_state_formula() const override { return false; }
};

class LogicOperator : public Formula {
   public:
    using Formula::Formula;

    bool is_a_state_formula() const override {
        for (const auto& f : subformulas) {
            if (!f->is_a_state_formula()) {
                return false;
            }
        }
        return true;
    }
};

namespace CTL {

// ############ Define Const Formulas #################

class Bool : public Formula {
   public:
    bool val;
    Bool(bool val) : Formula(OpCode::Bool, {}, {"true", "false"}), val(val) {}
    std::shared_ptr<Bool> clone() const {
        return std::make_shared<CTL::Bool>(val);
    }
    std::string str() const override { return val ? "true" : "false"; }
    bool is_a_state_formula() const override { return true; }

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

// ############# Define LogicOperators #################

class Not : public LogicOperator {
   public:
    Not(std::shared_ptr<Formula> phi)
        : LogicOperator(OpCode::Not, {phi}, {"not", "~"}) {}
    std::string str() const override { return "not " + subformulas[0]->str(); }

    std::shared_ptr<Not> clone() const {
        return std::make_shared<Not>(subformulas[0]);
    }

    bool is_a_state_formula() const override {
        return subformulas[0]->is_a_state_formula();
    }

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class Or : public LogicOperator {
   public:
    Or(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : LogicOperator(OpCode::Or, {phi, psi}, {"or", "|"}) {}
    std::string str() const override {
        return "(" + subformulas[0]->str() + " or " + subformulas[1]->str() +
               ")";
    }
    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class And : public Formula {
   public:
    And(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : Formula(OpCode::And, {phi, psi}, {"and", "&"}) {}
    std::string str() const override {
        return "(" + subformulas[0]->str() + " and " + subformulas[1]->str() +
               ")";
    }
    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class Imply : public Formula {
   public:
    Imply(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : Formula(OpCode::Imply, {phi, psi}, {"->"}) {}
    std::string str() const override {
        return "(" + subformulas[0]->str() + " -> " + subformulas[1]->str() +
               ")";
    }
    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

// ######## Define PathQuantifiers #########

class E : public PathQuantifier {
   public:
    E(std::shared_ptr<Formula> phi) : PathQuantifier(OpCode::E, {phi}, {"E"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class A : public PathQuantifier {
   public:
    A(std::shared_ptr<Formula> phi) : PathQuantifier(OpCode::A, {phi}, {"A"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

// ########## Define TemporalOperators ############

class X : public TemporalOperator {
   public:
    X(std::shared_ptr<Formula> phi)
        : TemporalOperator(OpCode::X, {phi}, {"X"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
};

class U : public TemporalOperator {
   public:
    U(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : TemporalOperator(OpCode::U, {phi, psi}, {"U"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class R : public TemporalOperator {
   public:
    R(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : TemporalOperator(OpCode::R, {phi, psi}, {"R"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class F : public TemporalOperator {
   public:
    F(std::shared_ptr<Formula> phi)
        : TemporalOperator(OpCode::F, {phi}, {"F"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

class G : public TemporalOperator {
   public:
    G(std::shared_ptr<Formula> phi)
        : TemporalOperator(OpCode::G, {phi}, {"G"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
};

// ######### Define Atomic Proposition ##########

class AtomicProposition : public Formula {
   public:
    std::string name;
    AtomicProposition(std::string name)
        : Formula(OpCode::Atomic, {}, {}), name(name) {}

    std::shared_ptr<AtomicProposition> clone() const {
        return std::make_shared<AtomicProposition>(name);
    }
    std::string str() const override { return name; }

    std::shared_ptr<Formula> get_equivalent_restricted_formula() const override;
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override;
    bool is_a_state_formula() const override { return true; }
};

// ############ Define Compositional Operators

inline std::shared_ptr<Formula> AX(std::shared_ptr<Formula> formula) {
    return std::make_shared<A>(std::make_shared<X>(formula));
}

inline std::shared_ptr<Formula> EX(std::shared_ptr<Formula> formula) {
    return std::make_shared<E>(std::make_shared<X>(formula));
}

inline std::shared_ptr<Formula> AF(std::shared_ptr<Formula> formula) {
    return std::make_shared<A>(std::make_shared<F>(formula));
}

inline std::shared_ptr<Formula> EF(std::shared_ptr<Formula> formula) {
    return std::make_shared<E>(std::make_shared<F>(formula));
}

inline std::shared_ptr<Formula> EG(std::shared_ptr<Formula> formula) {
    return std::make_shared<E>(std::make_shared<G>(formula));
}

inline std::shared_ptr<Formula> AU(std::shared_ptr<Formula> formula) {
    return std::make_shared<A>(std::make_shared<U>(formula));
}

inline std::shared_ptr<Formula> EU(std::shared_ptr<Formula> formula) {
    return std::make_shared<E>(std::make_shared<U>(formula));
}

inline std::shared_ptr<Formula> AR(std::shared_ptr<Formula> psi,
                                   std::shared_ptr<Formula> phi) {
    return std::make_shared<A>(std::make_shared<R>(psi, phi));
}

inline std::shared_ptr<Formula> ER(std::shared_ptr<Formula> psi,
                                   std ::shared_ptr<Formula> phi) {
    return std::make_shared<E>(std::make_shared<R>(psi, phi));
}

// ############ Define Equivalent Formulas ########

inline std::shared_ptr<Formula> LNot(std::shared_ptr<Formula> formula) {
    if (formula->opcode == OpCode::Not) {
        if (formula->subformulas[0]->opcode == OpCode::Not) {
            return LNot(formula->subformulas[0]->subformulas[0]);
        }
        return formula->subformulas[0];
    }
    return std::make_shared<CTL::Not>(formula);
}

inline std::shared_ptr<Formula> CTL::Bool::get_equivalent_restricted_formula()
    const {
    return clone();
}
inline std::shared_ptr<Formula> CTL::Bool::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return clone();
}

inline std::shared_ptr<Formula> CTL::Not::get_equivalent_restricted_formula()
    const {
    return LNot(subformulas[0]->get_equivalent_restricted_formula());
}
inline std::shared_ptr<Formula> CTL::Not::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<Not>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::Or::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::Or>(
        subformulas[0]->get_equivalent_restricted_formula(),
        subformulas[1]->get_equivalent_restricted_formula());
}
inline std::shared_ptr<Formula> CTL::Or::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<Or>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP),
        subformulas[1]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::And::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::Not>(std::make_shared<CTL::Or>(
        LNot(subformulas[0]->get_equivalent_restricted_formula()),
        LNot(subformulas[1]->get_equivalent_restricted_formula())));
}
inline std::shared_ptr<Formula> CTL::And::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<And>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP),
        subformulas[1]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::Imply::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::Or>(
        LNot(subformulas[0]->get_equivalent_restricted_formula()),
        subformulas[1]->get_equivalent_restricted_formula());
}
inline std::shared_ptr<Formula> CTL::Imply::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<Imply>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP),
        subformulas[1]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::E::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::E>(
        subformulas[0]->get_equivalent_restricted_formula());
}

inline std::shared_ptr<Formula> CTL::E::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    std::shared_ptr<Formula> sf =
        subformulas[0]->get_equivalent_non_fair_formula(fairAP);
    return std::make_shared<CTL::E>(
        LNot(std::make_shared<CTL::And>(fairAP, sf)));
}

inline std::shared_ptr<Formula> CTL::A::get_equivalent_restricted_formula()
    const {
    std::shared_ptr<Formula> p_formula = subformulas[0];
    std::shared_ptr<Formula> sf0 =
        p_formula->subformulas[0]->get_equivalent_restricted_formula();
    std::shared_ptr<Formula> neg_sf0 = LNot(sf0);

    switch (p_formula->opcode) {
        case (OpCode::X): {
            return std::make_shared<CTL::Not>(
                std::make_shared<CTL::E>(std::make_shared<CTL::X>(neg_sf0)));
        }
        case (OpCode::F): {
        }
    }

    return std::make_shared<CTL::Not>(std::make_shared<CTL::E>(
        LNot(subformulas[0]->get_equivalent_restricted_formula())));
}

inline std::shared_ptr<Formula> CTL::A::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    std::shared_ptr<Formula> sf =
        subformulas[0]->get_equivalent_non_fair_formula(fairAP);
    return std::make_shared<CTL::A>(
        LNot(std::make_shared<CTL::And>(LNot(sf), fairAP)));
}
inline std::shared_ptr<Formula> CTL::X::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::X>(
        subformulas[0]->get_equivalent_restricted_formula());
}

inline std::shared_ptr<Formula> CTL::U::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<U>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP),
        subformulas[1]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::U::get_equivalent_restricted_formula()
    const {
    return std::make_shared<U>(
        subformulas[0]->get_equivalent_restricted_formula(),
        subformulas[1]->get_equivalent_restricted_formula());
}

inline std::shared_ptr<Formula> CTL::R::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<R>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP),
        subformulas[1]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::R::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::Not>(std::make_shared<CTL::U>(
        LNot(subformulas[0]->get_equivalent_restricted_formula()),
        LNot(subformulas[1]->get_equivalent_restricted_formula())));
}

inline std::shared_ptr<Formula> CTL::F::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<F>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::F::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::U>(
        std::make_shared<CTL::Bool>(true),
        subformulas[0]->get_equivalent_restricted_formula());
}

inline std::shared_ptr<Formula> CTL::G::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<G>(
        subformulas[0]->get_equivalent_non_fair_formula(fairAP));
}

inline std::shared_ptr<Formula> CTL::G::get_equivalent_restricted_formula()
    const {
    return std::make_shared<CTL::Not>(std::make_shared<CTL::U>(
        std::make_shared<CTL::Bool>(true),
        LNot(subformulas[0]->get_equivalent_restricted_formula())));
}

inline std::shared_ptr<Formula>
CTL::AtomicProposition::get_equivalent_restricted_formula() const {
    return clone();
}
inline std::shared_ptr<Formula>
CTL::AtomicProposition::get_equivalent_non_fair_formula(
    std::shared_ptr<Formula> fairAP) const {
    return std::make_shared<CTL::And>(clone(), fairAP);
}

}  // namespace CTL
