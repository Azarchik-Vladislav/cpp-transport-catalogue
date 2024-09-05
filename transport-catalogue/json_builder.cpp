#include "json_builder.h"

using namespace std::literals;

namespace json {
//--------------------BUILDER----------------------
Builder::DictItemContext Builder::StartDict() {
    AddObject(Dict{}, /* one_shot */ false);
    return BaseContext{*this};
}

Builder::ArrayItemContext Builder::StartArray() {
    AddObject(Array{}, /* one_shot */ false);
    return BaseContext{*this};
}

Builder::DictValueContext Builder::Key(const std::string key) {
    Node::Value& host_value = GetCurrentValue();
    
    nodes_stack_.push_back(
        &std::get<Dict>(host_value)[std::move(key)]
    );

    return BaseContext{*this};
}

Builder::BaseContext Builder::Value(Node::Value value) {
    AddObject(std::move(value), /* one_shot */ true);
    return *this;
}

Builder::BaseContext Builder::EndDict() {
    nodes_stack_.pop_back();
    return *this;
}

Builder::BaseContext Builder::EndArray() {
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    return root_;
}

Node::Value& Builder::GetCurrentValue() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }
    return nodes_stack_.back()->GetValue();
}

const Node::Value& Builder::GetCurrentValue() const {
    return const_cast<Builder*>(this)->GetCurrentValue();
}

void Builder::AssertNewObjectContext() const {
    if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
        throw std::logic_error("New object in wrong context"s);
    }
}

void Builder::AddObject(Node::Value value, bool one_shot) {
    Node::Value& host_value = GetCurrentValue();

    if (std::holds_alternative<Array>(host_value)) {
        Node& node
            = std::get<Array>(host_value).emplace_back(std::move(value));

        if (!one_shot) {
            nodes_stack_.push_back(&node);
        }
    } else {
        AssertNewObjectContext();
        host_value = std::move(value);

        if (one_shot) {
            nodes_stack_.pop_back();
        }
    }
}

//--------------------DICTVALUECONTEXT----------------------

Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) {
    Builder::BaseContext::Value(value);
    return BaseContext {*this};
}

//--------------------ARRAYITEMCONTEXT----------------------
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    Builder::BaseContext::Value(value);
    return BaseContext {*this};
}
} // namespace json
    