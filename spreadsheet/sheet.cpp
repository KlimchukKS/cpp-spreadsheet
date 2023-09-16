#include "sheet.h"
#include "common.h"

#include <iostream>
#include <optional>
#include <stack>

Sheet::~Sheet() {}

bool Sheet::IsAcyclicGraph(Position start_vertex) {
    std::stack<Position> st;
    st.push(start_vertex);

    std::unordered_set<Position, PositionHasher> visited_vertex;

    while (!st.empty()) {
        Position pos = st.top(); // Получаем из стека очередную вершину.
        st.pop();

        if (!visited_vertex.count(pos)) {
            visited_vertex.insert(pos);
            // Если в таблице ещё нет вершины в позиции pos, то добавляем её в таблицу со значением EmptyImpl
            if (!sheet_values_.count(pos)) {
                SetCell(pos, "");
            }
            // Теперь добавляем в стек все непосещённые соседние вершины,
            // вместо вызова рекурсии перебираем смежные вершины.
            for (Position adjacent_vertex : sheet_values_[pos].get()->GetReferencedCells()) {
                if (!visited_vertex.count(adjacent_vertex)) { // Если вершина не посещена, то
                    st.push(adjacent_vertex);
                } else {
                    return false;
                }
            }
        }
    }

    return true;
}

void Sheet::ClearCache(Position start_vertex) {
    // Так как мы знаем, что в графе нет циклических зависимостей, то можно воспользоваться
    // алгоритмом обхода дерева в глубину, с использованием стека, чтобы избежать рекурсии

    std::stack<Position> st;
    st.push(start_vertex);

    while (!st.empty()) {
        Position pos = st.top();
        st.pop();

        // Очищаем содержимое кэша
        sheet_values_[pos].get()->ClearCache();

        // Добавляем в стек все вершины, которые ссылались на данную ячейку
        for (Position adjacent_vertex : sheet_values_[pos].get()->GetVertexInverseDependence()) {
            st.push(adjacent_vertex);
        }
    }
}

void Sheet::UpdateDependencies(Position start_vertex) {
    // Сообщаем всем вершинам прямой последовательности о том,
    // что данная вершина на них теперь ссылается либо, что прекращает ссылаться

    auto referenced_cells = sheet_values_[start_vertex].get()->GetReferencedCells();
    std::unordered_set<Position, CellHasher::PositionHasher> new_direct_dependence(referenced_cells.begin(), referenced_cells.end());

    auto& old_direct_dependence = sheet_values_[start_vertex].get()->GetVertexDirectDependence();

    // Сообщаем вершинам о том, что на них теперь ссылается эта вершина
    for (auto pos : new_direct_dependence) {
        if (!old_direct_dependence.count(pos)) {
            sheet_values_[pos]->GetVertexInverseDependence().insert(start_vertex);
        }
    }
    // Сообщаем вершинам о том, что эта вершина на них больше не ссылается
    for (auto pos : old_direct_dependence) {
        if (!new_direct_dependence.count(pos) && sheet_values_[pos]->GetVertexInverseDependence().count(pos)) {
            sheet_values_[pos]->GetVertexInverseDependence().erase(start_vertex);
        }
    }

    old_direct_dependence = std::move(new_direct_dependence);
}

void Sheet::ClearDependencies(Position start_vertex) {
    auto& direct_dependence = sheet_values_[start_vertex].get()->GetVertexDirectDependence();

    // Сообщаем вершинам о том, что эта вершина на них больше не ссылается
    for (auto pos : direct_dependence) {
        if (sheet_values_[pos]->GetVertexInverseDependence().count(pos)) {
            sheet_values_[pos]->GetVertexInverseDependence().erase(pos);
        }
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    minimum_printable_area_.rows = (pos.row > minimum_printable_area_.rows - 1) ? pos.row + 1 : minimum_printable_area_.rows;
    minimum_printable_area_.cols = (pos.col > minimum_printable_area_.cols - 1) ? pos.col + 1 : minimum_printable_area_.cols;

    std::unique_ptr<Cell> tmp;

    if (sheet_values_[pos]) {
        tmp = std::move(sheet_values_[pos]);
        sheet_values_[pos] = std::make_unique<Cell>(*this);
        sheet_values_[pos].get()->Set(text);
    } else {
        sheet_values_[pos] = std::make_unique<Cell>(*this);
        sheet_values_[pos].get()->Set(text);
    }

    if (!IsAcyclicGraph(pos)) {
        std::swap(tmp, sheet_values_[pos]);
        throw CircularDependencyException("Circular dependency");
    }

    if (sheet_values_[pos]->IsHaveCache()) {
        ClearCache(pos);
    }

    UpdateDependencies(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (sheet_values_.count(pos)) {
        return (const CellInterface*)sheet_values_.at(pos).get();
    }

    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (sheet_values_.count(pos)) {
        return (CellInterface*)sheet_values_.at(pos).get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (sheet_values_.count(pos)) {
        ClearCache(pos);
        ClearDependencies(pos);

        sheet_values_.erase(pos);

        if (pos.col == minimum_printable_area_.cols - 1 || pos.row == minimum_printable_area_.rows - 1) {
            Size tmp;
            for (const auto& [p, cell] : sheet_values_) {
                tmp.rows = (p.row > tmp.rows - 1) ? p.row + 1 : tmp.rows;
                tmp.cols = (p.col > tmp.cols - 1) ? p.col + 1 : tmp.cols;
            }
            std::swap(minimum_printable_area_, tmp);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return minimum_printable_area_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < minimum_printable_area_.rows; ++i) {
        for (int j = 0; j < minimum_printable_area_.cols; ++j) {
            if (sheet_values_.count({i, j})) {
                const auto value = sheet_values_.at({i, j})->GetValue();

                if (std::holds_alternative<FormulaError>(value)) {
                    output << std::get<FormulaError>(value);;
                } else if (std::holds_alternative<double>(value)) {
                    output << std::get<double>(value);
                } else {
                    output << std::get<std::string>(value);
                }
            }

            if (j + 1 != minimum_printable_area_.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < minimum_printable_area_.rows; ++i) {
        for (int j = 0; j < minimum_printable_area_.cols; ++j) {
            if (sheet_values_.count({i, j})) {
                output << sheet_values_.at({i, j})->GetText();
            }
            if (j + 1 != minimum_printable_area_.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
