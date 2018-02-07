/********************************************************************************

 classes.h - Contains definitions of 'rule' and 'symbols' classes, and the
             bodies of some of their methods.
              
 ********************************************************************************/

#ifndef SEQUITERCLASSES_H
#define SEQUITERCLASSES_H

#include <assert.h>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <memory.h> // for memset
#include <stdlib.h> // for malloc

std::ostream &operator << (std::ostream &o, symbols &s);

extern symbols **find_digram(symbols *s);     // defined in classes.cc

///////////////////////////////////////////////////////////////////////////

class rules {

   // the guard node in the linked list of symbols that make up the rule
   // It points forward to the first symbol in the rule, and backwards
   // to the last symbol in the rule. Its own value points to the rule data 
   // structure, so that symbols can find out which rule they're in  
   symbols *guard;

   // count keeps track of the number of times the rule is used in the grammar
   int count;

   // Usage stores the number of times a rule is used in the input.
   //    An example of the difference between count and Usage: in a grammar
   //    with rules S->abXcdXef , X->gAiAj , A->kl , rule A's count is 2
   //    (it is used two times in the grammar), while its Usage is 4 (there
   //    are two X's in the input sequence, and each of them uses A two times)
   int Usage;

   // number can serve two purposes:
   // (1) numbering the rules nicely for printing (in this case it's not essential for the algorithm)
   // (2) if this is a non-terminal symbol, assign a code to it (used in compression and forget_print())
   int number;

public:
   void output();     // output right hand of the rule, when printing out grammar
   void output2();    // output right hand of the rule, when compressing

   rules();
   ~rules();

   void reuse() { count ++; }
   void deuse() { count --; }

   symbols *first();     // pointer to first symbol of rule's right hand
   symbols *last();      // pointer to last symbol of rule's right hand

   int freq()           { return count; }
   int usage()          { return Usage; }
   void usage(int i)    { Usage += i; }
   int index()          { return number; }
   void index(int i)    { number = i; }

   void reproduce();    // reproduce full expansion of the rule
};

class symbols {
  symbols *n, *p;     // next and previous symbol within the rule
  ulong s;            // symbol value (e.g. ASCII code, or rule index)

public:
   ulong get_s(void) { return s; }

   // print out symbol, or, if it is non-terminal, rule's full expansion
   void reproduce() {
      if (nt()) rule()->reproduce();
      else cout << *this;
   }

   // initializes a new terminal symbol
   symbols(ulong sym) {
      s = sym * 2 + 1; // an odd number, so that they're a distinct
                        // space from the rule pointers, which are 4-byte aligned
      p = n = 0;
      num_symbols ++;
   }

   // initializes a new symbol to refer to a rule, and increments the reference
   // count of the corresponding rule
   symbols(rules *r) {
      s = (ulong) r;
      p = n = 0;
      rule()->reuse();
      num_symbols ++;
   }

   // links two symbols together, removing any old digram from the hash table
   static void join(symbols *left, symbols *right) {
      if (left->n) {
         left->delete_digram();

         // This is to deal with triples, where we only record the second
         // pair of the overlapping digrams. When we delete the second pair,
         // we insert the first pair into the hash table so that we don't
         // forget about it.  e.g. abbbabcbb

         if (right->p && right->n &&
            right->value() == right->p->value() &&
            right->value() == right->n->value()) {
         *find_digram(right) = right;
         }

         if (left->p && left->n &&
            left->value() == left->n->value() &&
            left->value() == left->p->value()) {
         *find_digram(left->p) = left->p;
         }
      }
      left->n = right; right->p = left;
   }

   // cleans up for symbol deletion: removes hash table entry and decrements
   // rule reference count
   ~symbols() {
      join(p, n);
      if (!is_guard()) {
         delete_digram();
         if (nt()) rule()->deuse(); 
      }
      num_symbols --;
   }

   // inserts a symbol after this one.
   void insert_after(symbols *y) {
      join(y, n);
      join(this, y);
   }

   // removes the digram from the hash table
   void delete_digram() {
      if (is_guard() || n->is_guard()) return;
      symbols **m = find_digram(this);
      if (m == 0) return;
      for (int i = 0; i < K; i ++) if (m[i] == this) m[i] = (symbols *) 1;
   }

   // is_guard() returns true if this is the guard node marking the beginning/end of a rule
   bool is_guard() { return nt() && rule()->first()->prev() == this; }

   // nt() returns true if a symbol is non-terminal. 
   // We make sure that terminals have odd-numbered values.
   // Because the value of a non-terminal is a pointer to
   // the corresponding rule, they're guaranteed to be even
   // (more -- a multiple of 4) on modern architectures

   int nt() { return ((s % 2) == 0) && (s != 0);}

   symbols *next() { return n; }
   symbols *prev() { return p; }
   inline ulong raw_value() { return s; }
   inline ulong value() { return s / 2; }

   // assuming this is a non-terminal, rule() returns the corresponding rule
   rules *rule() { return (rules *) s; }

   void substitute(rules *r);        // substitute digram with non-terminal symbol

   int check();                      // check digram and enforce the Sequitur constraints if necessary

   void expand();                    // substitute non-terminal symbol with its rule's right hand

   void point_to_self() { join(this, this); }

};


/********************************************************************************

 classes.cc - Module containing (part of the) methods of 'rules' and 'symbols'
              classes, and functions for working with the hash table of digrams
              printing out the grammar.
              
 Notes:
    For the rest of 'symbols' and 'rules' methods, see classes.h .

 ********************************************************************************/

rules::rules() {
  num_rules ++;
  guard = new symbols(this);
  guard->point_to_self();
  count = number = Usage = 0;
}

rules::~rules() { 
  num_rules --;
  delete guard;
}

symbols *rules::first()
{
   return guard->next();
}   // pointer to first symbol of rule's right hand

symbols *rules::last()
{
   return guard->prev();
}   // pointer to last symbol of rule's right hand

// ***********************************************************************************
// symbols::check()
//    check digram made of this symbol and the symbol following it,
//    and enforce Sequitur constraints.
//
// Return values
//    0 : did not change the grammar (there was no violation of contraints)
//    1 : did change the grammar (there were violations of contraints)
//
// Global variables used
//    K (minimum number of times a digram must occur to form rule)
// ***********************************************************************************
int symbols::check() {
  if (is_guard() || n->is_guard()) return 0;

  symbols **x = find_digram(this);
  if (!x) return 0;    // if either symbol of the digram is a delimiter -> do nothing

  int i;
  
  // if digram is not yet in the hash table -> put it there, and return
  for (i = 0; i < K; i ++)
    if (int(x[i]) <= 1) {
      x[i] = this;
      return 0;
    }
  
  // if repetitions overlap -> do nothing
  for (i = 0; i < K; i ++) 
    if (x[i]->next() == this || next() == x[i])
      return 0;
  
  rules *r;
  
  // reuse an existing rule

  for (i = 0; i < K; i ++)
    if (x[i]->prev()->is_guard() && x[i]->next()->next()->is_guard()) {
      r = x[i]->prev()->rule();
      substitute(r); 
      
      // check for an underused rule

      if (r->first()->nt() && r->first()->rule()->freq() == 1) r->first()->expand();
      // if (r->last()->nt() && r->last()->rule()->freq() == 1) r->last()->expand();

      return 1;
    }

  symbols *y[100];
  for (i = 0; i < K; i ++) y[i] = x[i];
  
  // make a copy of the pointers to digrams,
  // so that they don't change under our feet
  // especially when we create this replacement rule

  // create a new rule
  
  r = new rules;

  if (nt()) 
    r->last()->insert_after(new symbols(rule()));
  else 
    r->last()->insert_after(new symbols(value()));

  if (next()->nt()) 
    r->last()->insert_after(new symbols(next()->rule()));
  else
    r->last()->insert_after(new symbols(next()->value()));
  
  for (i = 0; i < K; i ++) {
    if (y[i] == r->first()) continue;
    // check that this hasn't been deleted
    bool deleted = 1;
    for (int j = 0; j < K; j ++)
      if (y[i] == x[j]) {
        deleted = 0;
        break;
      }
    if (deleted) continue;

    y[i]->substitute(r);
    //    y[i] = (symbols *) 1; // should be x
  }
  
  x[0] = r->first();

  substitute(r);
  
  // check for an underused rule
  
  if (r->first()->nt() && r->first()->rule()->freq() == 1) r->first()->expand();
  //  if (r->last()->nt() && r->last()->rule()->freq() == 1) r->last()->expand();
  
  return 1;
}

// ***********************************************************************************
// symbols::expand()
//    This symbol is the last reference to its rule. It is deleted, and the
//    contents of the rule substituted in its place.
// ***********************************************************************************
void symbols::expand() {
  symbols *left = prev();
  symbols *right = next();
  symbols *f = rule()->first();
  symbols *l = rule()->last();

  symbols **m = find_digram(this);
  if (!m) return;
  delete rule();
  
  for (int i = 0; i < K; i ++)
    if (m[i] == this) m[i] = (symbols *) 1;

  s = 0; // if we don't do this, deleting the symbol tries to deuse the rule!

  delete this;

  join(left, f);
  join(l, right);

  *find_digram(l) = l;
}

// ***********************************************************************************
// symbols::substitute(rules *r)
//    Replace digram made up of this symbol and the symbol following it with
//    a non-terminal, which points to rule "r" (parameter).
// ***********************************************************************************
void symbols::substitute(rules *r)
{
  symbols *q = p;
  
  delete q->next();
  delete q->next();

  q->insert_after(new symbols(r));

  if (!q->check()) q->next()->check();
}


// ***********************************************************************************
// Hash table functions
//
//     Handle the hash table of digrams.
// ***********************************************************************************

// pick a prime! (large enough to hold every digram in the grammar, with room
// to spare

// #define PRIME 1000003
// #define PRIME 2000003
#define PRIME 4265561
// #define PRIME 12454987
// #define PRIME 24909791
// #define PRIME 62265551

// Standard open addressing or double hashing. See Knuth.

#define HASH(one, two) (((one) << 16 | (two)) % PRIME)
#define HASH2(one) (17 - ((one) % 17))

symbols **table = 0;

// ******************************************************************************
// symbols **find_digram(symbols *s)
//
//     Search hash table for digram made of symbols s->value() and
//     s->next()->value().
//
// Return value
//     - if digram found : Pointer to hash table element where digram is stored.
//     - otherwise       : 0
//
// Global variables used
//    delimiter  (symbol accross which not to form rules, see sequitur.cc)
// *******************************************************************************
symbols **find_digram(symbols *s)
{
  if (!table) {
    table = (symbols **) malloc(PRIME * K * sizeof(symbols *));
    memset(table, 0, PRIME * K * sizeof(symbols *));
  }

  ulong one = s->raw_value();
  ulong two = s->next()->raw_value();

  if (one == delimiter || two == delimiter) return 0;

  int jump = HASH2(one) * K;
  int insert = -1;
  int i = HASH(one, two) * K;

  while (1) {
    symbols *m = table[i];
    if (!m) {
      if (insert == -1) insert = i;
      return &table[insert];
    } 
    else if (int(m) == 1) insert = i;
    else if (m->raw_value() == one && m->next()->raw_value() == two) return &table[i];
    i = (i + jump) % PRIME;
  }
}

// ***********************************************************************************
// rules::reproduce()
//    Reproduce full expansion of a rule.
// ***********************************************************************************
void rules::reproduce()
{
  // for each symbol of the rule, call symbols::reproduce()!
  for (symbols *p = first(); !p->is_guard(); p = p->next())
    p->reproduce();
}


// ***********************************************************************************
// Overload operator << to write symbols of the grammar to streams,
//    in a formatted manner.
// ***********************************************************************************
std::ostream &operator << (std::ostream &o, symbols &s)
{
  extern int numbers;

  if (s.nt())
     o << s.rule()->index() << std::flush;
  else
	  o << '&' << s.value() << std::flush;

  return o;
}

// ***********************************************************************************
// rules::output()
//    Print right hand of this rule. If right hand contains non-terminals, descend
//    recursively and print right hands of subordinated rules, if these have not
//    been printed out yet.
//
//    Also, numbers the rule, which will indicate that right hand has been printed out.
//
// Global variables used
//    current_rule, print_rule_usage
// ***********************************************************************************
void rules::output() 
{
  symbols *s;
  
  for (s = first(); !s->is_guard(); s = s->next())
     if (s->nt() && s->rule()->index() == 0)
        s->rule()->output();

  //number = current_rule ++;

  for (s = first(); !s->is_guard(); s = s->next())
    cout << *s << ' ';
 
  cout << endl;
}

#endif
