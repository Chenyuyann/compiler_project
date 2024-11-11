#include "TypeCheck.h"

//global tabels
typeMap func2retType; // function name to return type

// global token ids to type
typeMap g_token2Type; 

// local token ids to type, since func param can override global param
typeMap funcparam_token2Type;
vector<typeMap*> local_token2Type;
std::unordered_map<string, int>arraylen;

paramMemberMap func2Param;
paramMemberMap struct2Members;


// private util functions
void error_print(std::ostream& out, A_pos p, string info)
{
    out << "Typecheck error in line " << p->line << ", col " << p->col << ": " << info << std::endl;
    exit(0);
}


void print_token_map(typeMap* map){
    for(auto it = map->begin(); it != map->end(); it++){
        std::cout << it->first << " : ";
        switch (it->second->type->type)
        {
        case A_dataType::A_nativeTypeKind:
            switch (it->second->type->u.nativeType)
            {
            case A_nativeType::A_intTypeKind:
                std::cout << "int";
                break;
            default:
                break;
            }
            break;
        case A_dataType::A_structTypeKind:
            std::cout << *(it->second->type->u.structType);
            break;
        default:
            break;
        }
        switch(it->second->isVarArrFunc){
            case 0:
                std::cout << " scalar";
                break;
            case 1:
                std::cout << " array";
                break;
            case 2:
                std::cout << " function";
                break;
        }
        std::cout << std::endl;
    }
}


void print_token_maps(){
    std::cout << "global token2Type:" << std::endl;
    print_token_map(&g_token2Type);
    std::cout << "local token2Type:" << std::endl;
    print_token_map(&funcparam_token2Type);
}


bool comp_aA_type(aA_type target, aA_type t){
    if(!target || !t)
        return false;
    if(target->type != t->type)
        return false;
    if(target->type == A_dataType::A_nativeTypeKind)
        if(target->u.nativeType != t->u.nativeType)
            return false;
    if(target->type == A_dataType::A_structTypeKind)
        if(target->u.structType != t->u.structType)
            return false;
    return true;
}


bool comp_tc_type(tc_type target, tc_type t){
    if(!target || !t)
        return false;
    
    // arr kind first
    if (target->isVarArrFunc && t->isVarArrFunc == 0)
        return false;
    
    // if target type is nullptr, alwayse ok
    return comp_aA_type(target->type, t->type);
}


tc_type tc_Type(aA_type t, uint isVarArrFunc){
    tc_type ret = new tc_type_;
    ret->type = t;
    ret->isVarArrFunc = isVarArrFunc;
    return ret;
}


tc_type tc_Type(aA_varDecl vd){
    if(vd->kind == A_varDeclType::A_varDeclScalarKind)
        return tc_Type(vd->u.declScalar->type, 0);
    else if(vd->kind == A_varDeclType::A_varDeclArrayKind)
        return tc_Type(vd->u.declArray->type, 1);
    return nullptr;
}


// public functions
void check_Prog(std::ostream& out, aA_program p)
{
    for (auto ele : p->programElements)
    {
        if(ele->kind == A_programVarDeclStmtKind){
            check_VarDecl(out, ele->u.varDeclStmt);
        }else if (ele->kind == A_programStructDefKind){
            check_StructDef(out, ele->u.structDef);
        }
    }
    
    for (auto ele : p->programElements){
        if(ele->kind == A_programFnDeclStmtKind){
            check_FnDeclStmt(out, ele->u.fnDeclStmt);
        }
        else if (ele->kind == A_programFnDefKind){
            check_FnDecl(out, ele->u.fnDef->fnDecl);
        }
    }

    for (auto ele : p->programElements){
        if(ele->kind == A_programFnDefKind){
            check_FnDef(out, ele->u.fnDef);
        }
        else if (ele->kind == A_programNullStmtKind){
            // do nothing
        }
    }

    out << "Typecheck passed!" << std::endl;
    return;
}

tc_type check_rightVal(std::ostream& out, aA_rightVal rightVal)
{
    if (rightVal == nullptr)
        return nullptr;
    switch (rightVal->kind){
        case A_rightValType::A_arithExprValKind:{
            return check_ArithExpr(out, rightVal->u.arithExpr);
        }
            break;
        case A_rightValType::A_boolExprValKind:{
            check_BoolExpr(out, rightVal->u.boolExpr);
        }
            break;
        default:
            break;
    }
    return nullptr;
}

void check_VarDecl(std::ostream& out, aA_varDeclStmt vd)
{
    if (!vd)
        return;
    string name;
    if (vd->kind == A_varDeclStmtType::A_varDeclKind){
        // decl only
        aA_varDecl vdecl = vd->u.varDecl;
        if(vdecl->kind == A_varDeclType::A_varDeclScalarKind){ // scalar
            name = *vdecl->u.declScalar->id;
            /* fill code here*/
            if(g_token2Type.find(name) != g_token2Type.end()){ // 全局变量
                error_print(out, vd->pos, "This global scalar \"" + name + "\" is already defined!");
                return;
            }
            if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){ // 函数参数
                error_print(out, vd->pos, "Local variables duplicates with function params!");
                return;
            }
            for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){ // 局部变量
                if((*it)->find(name) != (*it)->end()){
                    error_print(out, vd->pos, "This local scalar \"" + name + "\" is already defined!");
                    return;
                }
            }
            if(local_token2Type.size() == 0)
                g_token2Type[name] = tc_Type(vdecl);
            else
                (*local_token2Type.back())[name] = tc_Type(vdecl);
        }else if (vdecl->kind == A_varDeclType::A_varDeclArrayKind){ // array
            name = *vdecl->u.declArray->id;
            /* fill code here*/
            if(g_token2Type.find(name) != g_token2Type.end()){
                error_print(out, vd->pos, "This global array \"" + name + "\" is already defined!");
                return;
            }
            if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
                error_print(out, vd->pos, "This function parameter \"" + name + "\" is already defined!");
                return;
            }
            for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
                if((*it)->find(name) != (*it)->end()){
                    error_print(out, vd->pos, "This local array \"" + name + "\" is already defined!");
                    return;
                }
            }
            if(local_token2Type.size() == 0)
                g_token2Type[name] = tc_Type(vdecl);
            else
                (*local_token2Type.back())[name] = tc_Type(vdecl);
            arraylen[name] = vdecl->u.declArray->len;
        }
    }
    else if (vd->kind == A_varDeclStmtType::A_varDefKind){
        // decl and def
        aA_varDef vdef = vd->u.varDef;
        if (vdef->kind == A_varDefType::A_varDefScalarKind){
            name = *vdef->u.defScalar->id;
            /* fill code here, allow omited type */
            if(g_token2Type.find(name) != g_token2Type.end()){
                error_print(out, vd->pos, "This global scalar \"" + name + "\" is already defined!");
                return;
            }
            if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
                error_print(out, vd->pos, "This function parameter \"" + name + "\" is already defined!");
                return;
            }
            for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
                if((*it)->find(name) != (*it)->end()){
                    error_print(out, vd->pos, "This local scalar \"" + name + "\" is already defined!");
                    return;
                }
            }
            tc_type rightType = check_rightVal(out, vdef->u.defScalar->val);
            if(local_token2Type.size() == 0)
                g_token2Type[name] = rightType;
            else
                (*local_token2Type.back())[name] = rightType;
        }else if (vdef->kind == A_varDefType::A_varDefArrayKind){
            name = *vdef->u.defArray->id;
            /* fill code here, allow omited type */
            if(g_token2Type.find(name) != g_token2Type.end()){
                error_print(out, vd->pos, "This global array \"" + name + "\" is already defined!");
                return;
            }
            if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
                error_print(out, vd->pos, "This function parameter \"" + name + "\" is already defined!");
                return;
            }
            for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
                if((*it)->find(name) != (*it)->end()){
                    error_print(out, vd->pos, "This local array \"" + name + "\" is already defined!");
                    return;
                }
            }
            if(local_token2Type.size() == 0)
                g_token2Type[name] = tc_Type(vdef->u.defArray->type, 1);
            else
                (*local_token2Type.back())[name] = tc_Type(vdef->u.defArray->type, 1);
            arraylen[name] = vdef->u.defArray->len;
        }
    }
    return;
}


void check_StructDef(std::ostream& out, aA_structDef sd)
{
    if (!sd)
        return;
    string name = *sd->id;
    if (struct2Members.find(name) != struct2Members.end())
        error_print(out, sd->pos, "This id is already defined!");
    struct2Members[name] = &(sd->varDecls);
    return;
}


void check_FnDecl(std::ostream& out, aA_fnDecl fd)
{
    if (!fd)
        return;
    string name = *fd->id;

    // if already declared, should match
    if (func2Param.find(name) != func2Param.end()){
        // is function ret val matches
        /* fill code here */
        if(!comp_aA_type(func2retType[name]->type, fd->type)){ // 函数返回值类型不匹配
            error_print(out, fd->pos, "The function return type does not match the given left type!");
            return;
        }
        // is function params matches decl
        /* fill code here */
        if(func2Param[name]->size() != fd->paramDecl->varDecls.size()){ // 参数个数不匹配
            error_print(out, fd->pos, "The function parameter number does not match!");
            return;
        }
        for(int i = 0; i < fd->paramDecl->varDecls.size(); i++){ // 参数类型不匹配
            if(!comp_tc_type(tc_Type((*func2Param[name])[i]), tc_Type(fd->paramDecl->varDecls[i]))){ // 参数类型不匹配
                error_print(out, fd->pos, "The function parameter type does not match!");
            }
        }
    }else{
        // if not defined
        /* fill code here */
        func2Param[name] = &(fd->paramDecl->varDecls);
        func2retType[name] = tc_Type(fd->type, 2);
    }
    return;
}


void check_FnDeclStmt(std::ostream& out, aA_fnDeclStmt fd)
{
    if (!fd)
        return;
    check_FnDecl(out, fd->fnDecl);
    return;
}


void check_FnDef(std::ostream& out, aA_fnDef fd)
{
    if (!fd)
        return;
    // should match if declared
    check_FnDecl(out, fd->fnDecl);
    // add params to local tokenmap, func params override global ones
    funcparam_token2Type.clear(); // 清空函数参数表！
    for (aA_varDecl vd : fd->fnDecl->paramDecl->varDecls)
    {
        /* fill code here */
        if(g_token2Type.find(*vd->u.declScalar->id) != g_token2Type.end()){ // 全局变量重复定义
            error_print(out, vd->pos, "duplicate definition.");
            return;
        }
        else if(funcparam_token2Type.find(*vd->u.declScalar->id) != funcparam_token2Type.end()){ // 函数参数重复定义
            error_print(out, fd->pos, "duplicate definition.");
            return;
        }
        funcparam_token2Type[*vd->u.declScalar->id] = tc_Type(vd);
    }

    /* fill code here */
    typeMap local_token2Type_map;
    local_token2Type_map.clear();
    local_token2Type.push_back(&local_token2Type_map); // 进入函数作用域
    for (aA_codeBlockStmt stmt : fd->stmts)
    {
        check_CodeblockStmt(out, stmt);
        // return value type should match
        /* fill code here */        
    }
    local_token2Type.pop_back();
    return;
}


void check_CodeblockStmt(std::ostream& out, aA_codeBlockStmt cs){
    if(!cs)
        return;
    // variables declared in a code block should not duplicate with outer ones.
    switch (cs->kind)
    {
    case A_codeBlockStmtType::A_varDeclStmtKind:
        check_VarDecl(out, cs->u.varDeclStmt);
        break;
    case A_codeBlockStmtType::A_assignStmtKind:
        check_AssignStmt(out, cs->u.assignStmt);
        break;
    case A_codeBlockStmtType::A_ifStmtKind:
        check_IfStmt(out, cs->u.ifStmt);
        break;
    case A_codeBlockStmtType::A_whileStmtKind:
        check_WhileStmt(out, cs->u.whileStmt);
        break;
    case A_codeBlockStmtType::A_callStmtKind:
        check_CallStmt(out, cs->u.callStmt);
        break;
    case A_codeBlockStmtType::A_returnStmtKind:
        check_ReturnStmt(out, cs->u.returnStmt);
        break;
    default:
        break;
    }
    return;
}


void check_AssignStmt(std::ostream& out, aA_assignStmt as){
    if(!as)
        return;
    string name;
    tc_type deduced_type; // deduced type if type is omitted at decl
    // 如果在声明时省略了类型，则推导类型
    switch (as->leftVal->kind)
    {
        case A_leftValType::A_varValKind:{ // 左值是变量
            name = *as->leftVal->u.id;
            /* fill code here */
            int flag = 0;
            if(g_token2Type.find(name) != g_token2Type.end()){
                deduced_type = g_token2Type[name];
                flag = 1;
            }
            else if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
                deduced_type = funcparam_token2Type[name];
                flag = 1;
            }
            else if(func2Param.find(name) != func2Param.end()){ // 不可以给函数名称赋值
                error_print(out, as->pos, "Cannot assign a value to function!");
            }
            else{
                for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
                    if((*it)->find(name) != (*it)->end()){
                        deduced_type = (*(*it))[name];
                        flag = 1;
                        break;
                    }
                }
            }
            if(!flag){
                error_print(out, as->pos, "This id is not defined!");
                return;
            }
            switch(as->rightVal->kind){ // 检查右值类型
                case A_rightValType::A_arithExprValKind:{
                    check_ArithExpr(out, as->rightVal->u.arithExpr);
                }
                    break;
                case A_rightValType::A_boolExprValKind:{
                    check_BoolExpr(out, as->rightVal->u.boolExpr);
                }
                    break;
                default:
                    break;
            }
        }
            break;
        case A_leftValType::A_arrValKind:{ // 左值是数组
            /* fill code here */
            check_ArrayExpr(out, as->leftVal->u.arrExpr);
            switch(as->rightVal->kind){
                case A_rightValType::A_arithExprValKind:{
                    check_ArithExpr(out, as->rightVal->u.arithExpr);
                }
                    break;
                case A_rightValType::A_boolExprValKind:{
                    check_BoolExpr(out, as->rightVal->u.boolExpr);
                }
                    break;
                default:
                    break;
            }
        }
            break;
        case A_leftValType::A_memberValKind:{ // 左值是成员
            /* fill code here */
            check_MemberExpr(out, as->leftVal->u.memberExpr);
            switch(as->rightVal->kind){
                case A_rightValType::A_arithExprValKind:{
                    check_ArithExpr(out, as->rightVal->u.arithExpr);
                }
                    break;
                case A_rightValType::A_boolExprValKind:{
                    check_BoolExpr(out, as->rightVal->u.boolExpr);
                }
                    break;
                default:
                    break;
            }
        }
            break;
        default:    
            break;
    }
    return;
}


tc_type check_ArrayExpr(std::ostream& out, aA_arrayExpr ae){
    if(!ae)
        return nullptr;
    string name = *ae->arr->u.id;
    // check array name
    /* fill code here */
    int flag = 0;
    tc_type arrType;
    if(g_token2Type.find(name) != g_token2Type.end()){
        flag = 1;
        if(g_token2Type[name]->isVarArrFunc != 1){
            error_print(out, ae->pos, "It is not an array!");
            return nullptr;
        }
        arrType = g_token2Type[name];
    }
    else if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
        flag = 1;
        if(funcparam_token2Type[name]->isVarArrFunc != 1){
            error_print(out, ae->pos, "It is not an array!");
            return nullptr;
        }
        arrType = funcparam_token2Type[name];
    }
    else{
        for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
            if((*it)->find(name) != (*it)->end()){
                flag = 1;
                if((**it)[name]->isVarArrFunc != 1){
                    error_print(out, ae->pos, "It is not an array!");
                    return nullptr;
                }
                arrType = (**it)[name];
                // break;
            }
        }
    }
    if(!flag){
        error_print(out, ae->pos, "Array \"" + name + "\" is not defined!");
        return nullptr;
    }
        
    // check index
    /* fill code here */
    if(ae->idx->kind == A_indexExprKind::A_numIndexKind){
        if(arraylen[name] <= ae->idx->u.num){
            error_print(out, ae->pos, "Index is out of range!");
            return nullptr;
        }
    }
    else if(ae->idx->kind == A_indexExprKind::A_idIndexKind){
        string idx_name = *ae->idx->u.id;
        if(g_token2Type.find(idx_name) != g_token2Type.end()){
            if(g_token2Type[idx_name]->isVarArrFunc != 0){
                error_print(out, ae->pos, "Index should be a scalar!");
                return nullptr;
            }
        }
        else if(funcparam_token2Type.find(idx_name) != funcparam_token2Type.end()){
            if(funcparam_token2Type[idx_name]->isVarArrFunc != 0){
                error_print(out, ae->pos, "Index should be a scalar!");
                return nullptr;
            }
        }
        else{
            for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
                if((*it)->find(idx_name) != (*it)->end()){
                    if((**it)[idx_name]->isVarArrFunc != 0){
                        error_print(out, ae->pos, "Index should be a scalar!");
                        return nullptr;
                    }
                }
            }
        }
    }
    return arrType;
}

 
tc_type check_MemberExpr(std::ostream& out, aA_memberExpr me){
    // check if the member exists and return the tyep of the member
    if(!me)
        return nullptr;
    string name = *me->structId->u.id;
    // check struct name
    /* fill code here */
    int flag = 0;
    string structName;
    if(g_token2Type.find(name) != g_token2Type.end()){
        flag = 1;
        if(g_token2Type[name]->type->type != A_dataType::A_structTypeKind){
            error_print(out, me->pos, "The \"" + name + "\" is not a struct!");
            return nullptr;
        }
        structName = *(g_token2Type[name]->type->u.structType);
    }
    else if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
        flag = 1;
        if(funcparam_token2Type[name]->type->type != A_dataType::A_structTypeKind){
            error_print(out, me->pos, "The \"" + name + "\" is not a struct!");
            return nullptr;
        }
        structName = *(funcparam_token2Type[name]->type->u.structType);
    }
    else{
        for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
            if((*it)->find(name) != (*it)->end()){
                flag = 1;
                if((**it)[name]->type->type != A_dataType::A_structTypeKind){
                    error_print(out, me->pos, "The \"" + name + "\" is not a struct!");
                    return nullptr;
                }
                structName = *(**it)[name]->type->u.structType;
                break;
            }
        }
    }
    if(!flag){
        error_print(out, me->pos, "Struct \"" + name + "\" not defined!");
        return nullptr;
    }
    if(struct2Members.find(structName) == struct2Members.end()){ // 结构体名
        error_print(out, me->pos, "It is not a struct!");
        return nullptr;
    }
    // check member name
    /* fill code here */
    string member_name = *me->memberId;
    for(aA_varDecl vd : *struct2Members[structName]){
        if(vd->kind == A_varDeclType::A_varDeclScalarKind){
            if(*vd->u.declScalar->id == member_name){
                return tc_Type(vd);
            }
        }
        else if(vd->kind == A_varDeclType::A_varDeclArrayKind){
            if(*vd->u.declArray->id == member_name){
                return tc_Type(vd);
            }
        }
    }
    error_print(out, me->pos, "Struct Member \"" + member_name + "\" not defined!");
    return nullptr;
}


void check_IfStmt(std::ostream& out, aA_ifStmt is){
    if(!is)
        return;
    check_BoolUnit(out, is->boolUnit);
    /* fill code here, take care of variable scope */
    typeMap local_token2Type_if;
    local_token2Type_if.clear();
    local_token2Type.push_back(&local_token2Type_if); // 进入if语句作用域
    for(aA_codeBlockStmt s : is->ifStmts){
        check_CodeblockStmt(out, s);
    }
    local_token2Type.pop_back();
    
    /* fill code here */   
    typeMap local_token2Type_else;
    local_token2Type_else.clear();
    local_token2Type.push_back(&local_token2Type_else); // 进入else语句作用域 
    for(aA_codeBlockStmt s : is->elseStmts){
        check_CodeblockStmt(out, s);
    }
    local_token2Type.pop_back();
    /* fill code here */
    return;
}


void check_BoolExpr(std::ostream& out, aA_boolExpr be){
    if(!be)
        return;
    switch (be->kind)
    {
    case A_boolExprType::A_boolBiOpExprKind:
        check_BoolExpr(out, be->u.boolBiOpExpr->left);
        check_BoolExpr(out, be->u.boolBiOpExpr->right);
        break;
    case A_boolExprType::A_boolUnitKind:
        check_BoolUnit(out, be->u.boolUnit);
        break;
    default:
        break;
    }
    return;
}


void check_BoolUnit(std::ostream& out, aA_boolUnit bu){
    if(!bu)
        return;
    switch (bu->kind)
    {
        case A_boolUnitType::A_comOpExprKind:{ // 比较运算
            /* fill code here */
            tc_type leftType = check_ExprUnit(out, bu->u.comExpr->left);
            tc_type rightType = check_ExprUnit(out, bu->u.comExpr->right);
            string leftName, rightName;
            switch(leftType->type->type){
                case A_dataType::A_nativeTypeKind:
                    switch(leftType->type->u.nativeType){
                        case A_nativeType::A_intTypeKind:
                            leftName = "int";
                            break;
                        default:
                            break;
                    }
                    break;
                case A_dataType::A_structTypeKind:
                    leftName = *(leftType->type->u.structType);
                    break;
                default:
                    break;
            }
            switch(rightType->type->type){
                case A_dataType::A_nativeTypeKind:
                    switch(rightType->type->u.nativeType){
                        case A_nativeType::A_intTypeKind:
                            rightName = "int";
                            break;
                        default:
                            break;
                    }
                    break;
                case A_dataType::A_structTypeKind:
                    rightName = *(rightType->type->u.structType);
                    break;
                default:
                    break;
            }
            if(leftName != rightName){
                out << "Typecheck error in line " << bu->pos->line << ", col " << bu->pos->col << ": " << leftName << " is not comparable with " << rightName << "!" << std::endl;
                exit(0);
            }
        }
            break;
        case A_boolUnitType::A_boolExprKind: // 布尔表达式
            check_BoolExpr(out, bu->u.boolExpr);
            break;
        case A_boolUnitType::A_boolUOpExprKind: // 布尔一元运算
            check_BoolUnit(out, bu->u.boolUOpExpr->cond);
            break;
        default:
            break;
    }
    return;
}


tc_type check_ExprUnit(std::ostream& out, aA_exprUnit eu){ // 检查表达式单元
    // return the aA_type of expr eu
    if(!eu)
        return nullptr;
    tc_type ret;
    switch (eu->kind)
    {
        case A_exprUnitType::A_idExprKind:{ // 变量
            /* fill code here */
            string name = *eu->u.id;
            int flag = 0;
            if(g_token2Type.find(name) != g_token2Type.end()){
                flag = 1;
                ret = g_token2Type[name];
            }
            else if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
                flag = 1;
                ret = funcparam_token2Type[name];
            }
            else{
                for(auto it = local_token2Type.begin(); it != local_token2Type.end(); it++){
                    if((*it)->find(name) != (*it)->end()){
                        flag = 1;
                        ret = (**it)[name];
                        break;
                    }
                }
            }
            if(!flag){
                error_print(out, eu->pos, "This id is not defined!");
                return nullptr;
            }
        }
            break;
        case A_exprUnitType::A_numExprKind:{ // 数字
            aA_type numt = new aA_type_;
            numt->pos = eu->pos;
            numt->type = A_dataType::A_nativeTypeKind;
            numt->u.nativeType = A_nativeType::A_intTypeKind;
            ret = tc_Type(numt, 0);
        }
            break;
        case A_exprUnitType::A_fnCallKind:{ // 函数调用
            check_FuncCall(out, eu->u.callExpr);
            // check_FuncCall will check if the function is defined
            /* fill code here */
            ret = func2retType[*eu->u.callExpr->fn];
        }
            break;
        case A_exprUnitType::A_arrayExprKind:{ // 数组
            ret = check_ArrayExpr(out, eu->u.arrayExpr);
            /* fill code here */
        }
            break;
        case A_exprUnitType::A_memberExprKind:{ // 成员
            ret = check_MemberExpr(out, eu->u.memberExpr);
        }
            break;
        case A_exprUnitType::A_arithExprKind:{ // 算术表达式
            ret = check_ArithExpr(out, eu->u.arithExpr);
        }
            break;
        case A_exprUnitType::A_arithUExprKind:{ // 算术一元表达式
            ret = check_ExprUnit(out, eu->u.arithUExpr->expr);
        }
            break;
    }
    return ret;
}


tc_type check_ArithExpr(std::ostream& out, aA_arithExpr ae){ // 检查算术表达式
    if(!ae)
        return nullptr;
    tc_type ret;
    switch (ae->kind)
    {
        case A_arithExprType::A_arithBiOpExprKind:{
            ret = check_ArithExpr(out, ae->u.arithBiOpExpr->left);
            tc_type rightTyep = check_ArithExpr(out, ae->u.arithBiOpExpr->right);
            if(ret->type->type > 0 || ret->type->type != A_dataType::A_nativeTypeKind || ret->type->u.nativeType != A_nativeType::A_intTypeKind ||
            rightTyep->type->type > 0 || rightTyep->type->type != A_dataType::A_nativeTypeKind || rightTyep->type->u.nativeType != A_nativeType::A_intTypeKind)
                error_print(out, ae->pos, "Only int can be arithmetic expression operation values!");
        }
            break;
        case A_arithExprType::A_exprUnitKind:
            ret = check_ExprUnit(out, ae->u.exprUnit);
            break;
    }
    return ret;
}


void check_FuncCall(std::ostream& out, aA_fnCall fc){
    if(!fc)
        return;
    // check if function defined
    string func_name = *fc->fn;
    /* fill code here */
    if(func2Param.find(func_name) == func2Param.end()){
        error_print(out, fc->pos, "Function \"" + func_name + "\" is not defined!");
        return;
    }
        
    // check if parameter list matches
    if(fc->vals.size() != func2Param[func_name]->size()){
        error_print(out, fc->pos, "Function \"" + func_name + "\" parameter number does not match!");
        return;
    }
    for(int i = 0; i < fc->vals.size(); i++){
        /* fill code here */
        tc_type paramType = tc_Type((*func2Param[func_name])[i]);
        tc_type valType = check_rightVal(out, fc->vals[i]);
        if(!comp_tc_type(valType, paramType)){
            error_print(out, fc->vals[i]->pos, "Function \"" + func_name + "\" parameter type does not match!");
            return;
        }
    }
    return ;
}


void check_WhileStmt(std::ostream& out, aA_whileStmt ws){
    if(!ws)
        return;
    check_BoolUnit(out, ws->boolUnit);
    /* fill code here, take care of variable scope */
    typeMap local_token2Type_while;
    local_token2Type_while.clear();
    local_token2Type.push_back(&local_token2Type_while); // 进入while语句作用域
    for(aA_codeBlockStmt s : ws->whileStmts){
        check_CodeblockStmt(out, s);
    }
    /* fill code here */
    local_token2Type.pop_back();
    return;
}


void check_CallStmt(std::ostream& out, aA_callStmt cs){
    if(!cs)
        return;
    check_FuncCall(out, cs->fnCall);
    return;
}


void check_ReturnStmt(std::ostream& out, aA_returnStmt rs){
    if(!rs)
        return;
    check_rightVal(out, rs->retVal);
    return;
}

