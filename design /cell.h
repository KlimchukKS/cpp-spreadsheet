#pragma once

#include "common.h"
#include "formula.h"

#include <optional>

class Impl {
public:
    virtual ~Impl() = default;

    virtual std::string GetText() const = 0;
    virtual CellInterface::Value GetValue() const = 0;
};

class EmptyImpl : public Impl {
public:
    EmptyImpl() = default;

    std::string GetText() const override {
        return "";
    }

    CellInterface::Value GetValue() const override {
        return "";
    }
};

class TextImpl : public Impl {
public:
    TextImpl(std::string text)
    : value_(text) {}

    std::string GetText() const override {
        return value_;
    }

    CellInterface::Value GetValue() const override {
        return (!value_.empty() && value_[0] == '\'') ? value_.substr(1) : value_;
    }

private:
    std::string value_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string text) {
        value_ = ParseFormula(text);
    }

    std::string GetText() const override {
        return '=' + value_->GetExpression();
    }

    CellInterface::Value GetValue() const override {
        auto tmp = value_->Evaluate();

        if (std::holds_alternative<double>(tmp)) {
            return std::get<double>(tmp);
        }

        return std::get<FormulaError>(tmp);
    }

private:
    std::unique_ptr<FormulaInterface> value_;
};

class Cell : public CellInterface {
public:
    Cell(const SheetInterface& sheet);

    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
private:
    std::unique_ptr<Impl> impl_;

    const SheetInterface& sheet_;
    mutable std::optional<Value> cache_value_;
    std::unordered_set<Position> direct_dependence_;
    std::unordered_set<Position> inverse_dependence_;

    // обходим граф в глубину по inverse_dependence_
    void ClearCache();
};
