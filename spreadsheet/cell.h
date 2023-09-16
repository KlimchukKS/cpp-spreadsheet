#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

namespace CellHasher {
    struct PositionHasher {
        size_t operator() (Position pos) const {
            return pos.row + pos.col * 1000000007;
        }
    };
}

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);

    ~Cell();

    void Set(std::string text);

    void Clear();

    Value GetValue() const override;

    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    const std::unordered_set<Position, CellHasher::PositionHasher>& GetVertexInverseDependence() const;
    std::unordered_set<Position, CellHasher::PositionHasher>& GetVertexInverseDependence();

    const std::unordered_set<Position, CellHasher::PositionHasher>& GetVertexDirectDependence() const;
    std::unordered_set<Position, CellHasher::PositionHasher>& GetVertexDirectDependence();

    void ClearCache();

    bool IsHaveCache() const;

private:
    class Impl {
    public:
        virtual ~Impl() = default;

        virtual std::string GetText() const = 0;
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::vector<Position> GetReferencedCells() const {
            return {};
        }
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;

        std::string GetText() const override {
            return "";
        }

        CellInterface::Value GetValue() const override {
            return 0.0;
        }
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text)
                : value_(std::move(text)) {}

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
        FormulaImpl(std::string text, const Sheet &sheet)
        : value_(ParseFormula(text)), sheet_(sheet) {}

        std::string GetText() const override {
            return '=' + value_->GetExpression();
        }

        CellInterface::Value GetValue() const override {
            // На неявную конвертацию ругается компилятор
            auto tmp = value_->Evaluate((const SheetInterface&) sheet_);

            if (std::holds_alternative<double>(tmp)) {
                return std::get<double>(tmp);
            }

            return std::get<FormulaError>(tmp);
        }

        std::vector<Position> GetReferencedCells() const override {
            return value_->GetReferencedCells();
        }

    private:
        std::unique_ptr<FormulaInterface> value_;
        const Sheet& sheet_;
    };

    Sheet& sheet_;

    std::unique_ptr<Impl> impl_;

    mutable std::optional<Value> cache_value_;

    // Вершины, на которые ссылается данная ячейка
    std::unordered_set<Position, CellHasher::PositionHasher> direct_dependence_;
    // Вершины, которые ссылаются на эту ячейку
    std::unordered_set<Position, CellHasher::PositionHasher> inverse_dependence_;
};
