#include "../include/helpers.h"
#include "../include/structure_map.h"
#include "../include/logic.h"
#include "../include/builtin_funcs.h"
#include "test_macros.h"

using std::cout;
using std::endl;
using namespace cre;

void test_lit_groups(){

    ref<Var> x = new_var("x", cre_float);
    ref<Var> y = new_var("y", cre_float);
    ref<Var> z = new_var("z", cre_float);
    ref<Var> a = new_var("a", cre_float);
    ref<Var> b = new_var("b", cre_float);
    ref<Var> c = new_var("c", cre_float);
    ref<Var> d = new_var("d", cre_float);

    ref<Logic> c1 = AND(LessThan(x, y), LessThan(y, z), LessThan(y, z), ~Equals(z, x), ~Equals(y, 0));
    ref<Logic> c2 = AND(LessThan(a, b), LessThan(b, c), LessThan(b, c), LessThan(b, c), ~Equals(c, a), ~Equals(b, 0), ~Equals(d, 0));
    
    cout << c1 << endl;
    cout << c2 << endl;
    cout << "--------" << endl;
    auto group_set = logic_to_map_cands(c1, c2);
    auto& conj_pairs = group_set.conj_pairs; 
    for(auto conj_pair : conj_pairs){
        cout << "C:" << conj_pair.conj_ind_a << " " << conj_pair.conj_ind_b << " size:" << conj_pair.pairs.size() << endl;
        for(auto pair : conj_pair.pairs){
            cout << "P:(" << pair.index_a << "->" << pair.index_b << ") w: " << pair.weight << " n_vars: " << pair.n_vars << endl;
            for(size_t i = 0; i < pair.n_vars; i++){
                cout << pair.var_inds_a.inds[i] << "->" << pair.var_inds_b.inds[i] << endl;
            }
        }
    }

    ref<Logic> c1o = OR(AND(LessThan(x, y), ~Equals(z, x), ~Equals(y, 0)),
          AND(LessThan(x, y), Equals(z, x), ~Equals(y, 7)),
          AND(GreaterThan(x, y), ~Equals(z, x), ~Equals(y, 2))
         );
    ref<Logic> c2o = OR(AND(LessThan(a, b), Equals(c, a), ~Equals(b, 7), GreaterThan(d, 0)),
          AND(LessThan(a, b), ~Equals(c, a), ~Equals(b, 0)),
          AND(GreaterThan(a, b), ~Equals(c, a), ~Equals(b, 0), ~Equals(d, 7))
         );

    structure_map_logic(c1o, c2o);
    
    ref<Logic> auc = antiunify_logic(c1o, c2o);
    cout << "-------------------" << endl;
    cout << "auc: " << endl << auc << endl;



    // cout << "score: " << result.score << endl;
    // cout << "alignment: " << result.alignment << endl;
    // cout << "remap: " << result.remap << endl;

}


int main(){
    test_lit_groups();
    return 0;
}