#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

FormulaError::FormulaError(Category category)
: category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case FormulaError::Category::Ref:
            return "REF!"sv;
        case FormulaError::Category::Value:
            return "#VALUE!"sv;
        case FormulaError::Category::Div0:
            return "#DIV/0!"sv;
    }
    return "UNKNOWN ERROR"sv;
}

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
    try : ast_(ParseFormulaAST(expression)) {

    } catch (const std::exception& err) {
        throw FormulaException(err.what());
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            std::function<double(Position)> f = [&sheet](Position pos) {
                if(!pos.IsValid()){
                    throw FormulaError(FormulaError::Category::Ref);
                }

                if (const auto cell_ptr = sheet.GetCell(pos)) {
                    const auto& value = cell_ptr->GetValue();

                    if (std::holds_alternative<double>(value)) {
                        return std::get<double>(value);
                    } else if (std::holds_alternative<FormulaError>(value)) {
                        throw std::get<FormulaError>(value);
                    } else if (std::holds_alternative<std::string>(value)) {
                        const auto& str = std::get<std::string>(value);

                        size_t pos = 0;
                        double d = 0.0;

                        try {
                            d = std::stod(str, &pos);
                        } catch (...) {
                            throw FormulaError(FormulaError::Category::Value);
                        }

                        if (pos < str.size()) {
                            throw FormulaError(FormulaError::Category::Value);
                        }

                        return d;
                    }
                }

                return 0.0;
            };

            return ast_.Execute(f);
        } catch (const FormulaError& fe) {
            return fe;
        }
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        const auto& ref = ast_.GetCells();

        std::vector<Position> positions(ref.begin(), ref.end());
        std::sort(positions.begin(), positions.end());
        auto it = std::unique(positions.begin(), positions.end());
        positions.erase(it, positions.end());

        return positions;
    }

private:
    FormulaAST ast_;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
