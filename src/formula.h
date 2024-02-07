#pragma once
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

// ############ Define Const Formulas #################

class Bool : public Formula {
   public:
    bool val;
    Bool(bool val) : Formula(OpCode::Bool, {}, {"true", "false"}), val(val) {}

    std::shared_ptr<Bool> clone() const { return std::make_shared<Bool>(val); }
    std::string str() const override { return val ? "true" : "false"; }

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return clone();
    }
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return clone();
    }
    bool is_a_state_formula() const override { return true; }
};

std::shared_ptr<Formula> LNot(std::shared_ptr<Formula> formula);

// ############# Define LogicOperators #################

class Not : public LogicOperator {
   public:
    Not(std::shared_ptr<Formula> phi)
        : LogicOperator(OpCode::Not, {phi}, {"not", "~"}) {}
    std::string str() const override { return "not " + subformulas[0]->str(); }

    std::shared_ptr<Not> clone() const {
        return std::make_shared<Not>(subformulas[0]);
    }

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return LNot(subformulas[0]->get_equivalent_restricted_formula());
    }
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<Not>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP));
    }
    bool is_a_state_formula() const override {
        return subformulas[0]->is_a_state_formula();
    }
};

inline std::shared_ptr<Formula> LNot(std::shared_ptr<Formula> formula) {
    if (formula->opcode == OpCode::Not) {
        if (formula->subformulas[0]->opcode == OpCode::Not) {
            return LNot(formula->subformulas[0]->subformulas[0]);
        }
        return formula->subformulas[0];
    }
    return std::make_shared<class ::Not>(formula);
}

class Or : public LogicOperator {
   public:
    Or(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : LogicOperator(OpCode::Or, {phi, psi}, {"or", "|"}) {}
    std::string str() const override {
        return "(" + subformulas[0]->str() + " or " + subformulas[1]->str() +
               ")";
    }
    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::Or>(
            subformulas[0]->get_equivalent_restricted_formula(),
            subformulas[1]->get_equivalent_restricted_formula());
    }
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<Or>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP),
            subformulas[1]->get_equivalent_non_fair_formula(fairAP));
    }
};

class And : public Formula {
   public:
    And(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : Formula(OpCode::And, {phi, psi}, {"and", "&"}) {}
    std::string str() const override {
        return "(" + subformulas[0]->str() + " and " + subformulas[1]->str() +
               ")";
    }
    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::Not>(std::make_shared<class ::Or>(
            LNot(subformulas[0]->get_equivalent_restricted_formula()),
            LNot(subformulas[1]->get_equivalent_restricted_formula())));
    }
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<And>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP),
            subformulas[1]->get_equivalent_non_fair_formula(fairAP));
    }
};

class Imply : public Formula {
   public:
    Imply(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : Formula(OpCode::Imply, {phi, psi}, {"->"}) {}
    std::string str() const override {
        return "(" + subformulas[0]->str() + " -> " + subformulas[1]->str() +
               ")";
    }
    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::Or>(
            LNot(subformulas[0]->get_equivalent_restricted_formula()),
            subformulas[1]->get_equivalent_restricted_formula());
    }
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<Imply>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP),
            subformulas[1]->get_equivalent_non_fair_formula(fairAP));
    }
};

// ######## Define PathQuantifiers #########

class E : public PathQuantifier {
   public:
    E(std::shared_ptr<Formula> phi) : PathQuantifier(OpCode::E, {phi}, {"E"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class E>(
            subformulas[0]->get_equivalent_restricted_formula());
    }

    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        std::shared_ptr<Formula> sf =
            subformulas[0]->get_equivalent_non_fair_formula(fairAP);
        return std::make_shared<class ::E>(
            LNot(std::make_shared<class ::And>(fairAP, sf)));
    }
};

class A : public PathQuantifier {
   public:
    A(std::shared_ptr<Formula> phi) : PathQuantifier(OpCode::A, {phi}, {"A"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class Not>(std::make_shared<class E>(
            LNot(subformulas[0]->get_equivalent_restricted_formula())));
    }

    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        std::shared_ptr<Formula> sf =
            subformulas[0]->get_equivalent_non_fair_formula(fairAP);
        return std::make_shared<class ::A>(
            LNot(std::make_shared<class ::And>(LNot(sf), fairAP)));
    }
};

// ########## Define TemporalOperators ############

class X : public TemporalOperator {
   public:
    X(std::shared_ptr<Formula> phi)
        : TemporalOperator(OpCode::X, {phi}, {"X"}) {}

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::X>(
            subformulas[0]->get_equivalent_restricted_formula());
    }
};

class U : public TemporalOperator {
   public:
    U(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : TemporalOperator(OpCode::U, {phi, psi}, {"U"}) {}

    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<U>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP),
            subformulas[1]->get_equivalent_non_fair_formula(fairAP));
    }

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<U>(
            subformulas[0]->get_equivalent_restricted_formula(),
            subformulas[1]->get_equivalent_restricted_formula());
    }
};

class R : public TemporalOperator {
   public:
    R(std::shared_ptr<Formula> phi, std::shared_ptr<Formula> psi)
        : TemporalOperator(OpCode::R, {phi, psi}, {"R"}) {}

    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<R>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP),
            subformulas[1]->get_equivalent_non_fair_formula(fairAP));
    }

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::Not>(std::make_shared<class ::U>(
            LNot(subformulas[0]->get_equivalent_restricted_formula()),
            LNot(subformulas[1]->get_equivalent_restricted_formula())));
    }
};

class F : public TemporalOperator {
   public:
    F(std::shared_ptr<Formula> phi)
        : TemporalOperator(OpCode::F, {phi}, {"F"}) {}

    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<F>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP));
    }

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::U>(
            std::make_shared<class ::Bool>(true),
            subformulas[0]->get_equivalent_restricted_formula());
    }
};

class G : public TemporalOperator {
   public:
    G(std::shared_ptr<Formula> phi)
        : TemporalOperator(OpCode::G, {phi}, {"G"}) {}

    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<G>(
            subformulas[0]->get_equivalent_non_fair_formula(fairAP));
    }

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return std::make_shared<class ::Not>(std::make_shared<class ::U>(
            std::make_shared<class ::Bool>(true),
            LNot(subformulas[0]->get_equivalent_restricted_formula())));
    }
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

    std::shared_ptr<Formula> get_equivalent_restricted_formula()
        const override {
        return clone();
    }
    std::shared_ptr<Formula> get_equivalent_non_fair_formula(
        std::shared_ptr<Formula> fairAP) const override {
        return std::make_shared<class ::And>(clone(), fairAP);
    }
    bool is_a_state_formula() const override { return true; }
};

