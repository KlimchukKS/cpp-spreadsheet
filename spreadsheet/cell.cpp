#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell(Sheet& sheet)
: sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text[0] == '=') {
        impl_ = std::make_unique<FormulaImpl>(text.erase(0, 1), sheet_);
    } else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if(!cache_value_.has_value()){
        cache_value_ = impl_->GetValue();
    }

    return *cache_value_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !GetReferencedCells().empty();
}

void Cell::ClearCache() {
    cache_value_.reset();
}

bool Cell::IsHaveCache() const {
    return cache_value_.has_value();
}

const std::unordered_set<Position, CellHasher::PositionHasher>& Cell::GetVertexInverseDependence() const {
    return inverse_dependence_;
}

std::unordered_set<Position, CellHasher::PositionHasher>& Cell::GetVertexInverseDependence() {
    return inverse_dependence_;
}

const std::unordered_set<Position, CellHasher::PositionHasher>& Cell::GetVertexDirectDependence() const {
    return direct_dependence_;
}

std::unordered_set<Position, CellHasher::PositionHasher>& Cell::GetVertexDirectDependence() {
    return direct_dependence_;
}
