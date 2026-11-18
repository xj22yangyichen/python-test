// 你需要提交这份代码
#pragma once

#include "visitor.h"
#include <any>

struct calculator : visitor {
    using f80=double;
    std::any visit_num(num_node *p)override
    {
        return p->number;
    }
    template<typename T>
    std::any visit_op(node *l,node *r,T F)
    {
        auto lv=l->accept(this);
        auto rv=r->accept(this);
        if(std::any_cast<double>(&lv))
            return F(std::any_cast<double>(lv),std::any_cast<double>(rv));
        else
            return F(std::any_cast<long long>(lv),std::any_cast<long long>(rv));;
    }
    std::any visit_add(add_node *p)override
    {
        return visit_op(p->lnode,p->rnode,[](auto a,auto b){return a+b;});
    }
    std::any visit_mul(mul_node *p)override
    {
        return visit_op(p->lnode,p->rnode,[](auto a,auto b){return a*b;});
    }
    std::any visit_sub(sub_node *p)override
    {
        return visit_op(p->lnode,p->rnode,[](auto a,auto b){return a-b;});
    }
    std::any visit_div(div_node *p)override
    {
        return visit_op(p->lnode,p->rnode,[](auto a,auto b){return a/b;});
    }
    ~calculator() override = default;
};
