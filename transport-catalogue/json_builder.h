#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include"json.h"

namespace json {
class Builder {
    class BaseContext;
    class DictItemContext;
    class DictValueContext;
    class ArrayItemContext;
public:
    Builder() : root_(),
                nodes_stack_ {&root_} {

    }

    DictItemContext StartDict();
    ArrayItemContext StartArray();

    DictValueContext Key(const std::string key);

    BaseContext Value(Node::Value value);
    BaseContext EndDict();
    BaseContext EndArray();

    Node Build();
private:
    json::Node root_;
    std::vector<json::Node*> nodes_stack_;

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;

    void AssertNewObjectContext() const;
    void AddObject(Node::Value value, bool one_shot);

    class BaseContext{
    public:
        BaseContext(Builder& builder) : builder_(builder) {
        }

        DictItemContext StartDict() {
            return builder_.StartDict();
        }

        ArrayItemContext StartArray() {
            return builder_.StartArray();
        }

        DictValueContext Key(const std::string& key) {
            return builder_.Key(key);
        }

        BaseContext Value(Node::Value value) {
            return builder_.Value(value);
        }
        BaseContext EndDict() {
            return builder_.EndDict();
        }
        BaseContext EndArray() {
            return builder_.EndArray();
        }

        Node Build() {
            return builder_.Build();
        }
    private:
        Builder& builder_;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base) : BaseContext(base){
        }

        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        BaseContext Value(Node::Value value) = delete;
        BaseContext EndArray() = delete;

        Node Build() = delete;
    };

    class DictValueContext : public BaseContext {
    public:
        DictValueContext(BaseContext base) : BaseContext(base) {
        }

        DictItemContext Value(Node::Value value);

        DictValueContext Key() = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;

        Node Build() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base) : BaseContext(base) {
        }

        ArrayItemContext Value(Node::Value value);

        DictValueContext Key(const std::string& key) = delete;
        BaseContext EndDict() = delete;

        Node Build() = delete;
    };

};
}//namepace json