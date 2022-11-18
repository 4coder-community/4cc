

// Copyright 2012-2013 The Rust Project Developers. See the COPYRIGHT
// file at the top-level directory of this distribution and at
// http://rust-lang.org/COPYRIGHT.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

pub use self::BinOpToken::*;
pub use self::Nonterminal::*;
pub use self::DelimToken::*;
pub use self::Lit::*;
pub use self::Token::*;

use ast::{self};
use ptr::P;
use symbol::keywords;
use tokenstream::TokenTree;

use std::fmt;
use std::rc::Rc;

#[derive(Clone, RustcEncodable, RustcDecodable, PartialEq, Eq, Hash, Debug, Copy)]
pub enum BinOpToken {
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Caret,
    And,
    Or,
    Shl,
    Shr,
}

/// A delimiter token
#[derive(Clone, RustcEncodable, RustcDecodable, PartialEq, Eq, Hash, Debug, Copy)]
pub enum DelimToken {
    /// A round parenthesis: `(` or `)`
    Paren,
    /// A square bracket: `[` or `]`
    Bracket,
    /// A curly brace: `{` or `}`
    Brace,
    /// An empty delimiter
    NoDelim,
}

impl DelimToken {
    pub fn len(self) -> usize {
        if self == NoDelim { 0 } else { 1 }
    }
    
    pub fn is_empty(self) -> bool {
        self == NoDelim
    }
}

#[derive(Clone, RustcEncodable, RustcDecodable, PartialEq, Eq, Hash, Debug, Copy)]
pub enum Lit {
    Byte(ast::Name),
    Char(ast::Name),
    Integer(ast::Name),
    Float(ast::Name),
    Str_(ast::Name),
    StrRaw(ast::Name, usize), /* raw str delimited by n hash symbols */
    ByteStr(ast::Name),
    ByteStrRaw(ast::Name, usize), /* raw byte str delimited by n hash symbols */
}

impl Lit {
    pub fn short_name(&self) -> &'static str {
        match *self {
            Byte(_) => "byte",
            Char(_) => "char",
            Integer(_) => "integer",
            Float(_) => "float",
            Str_(_) | StrRaw(..) => "string",
            ByteStr(_) | ByteStrRaw(..) => "byte string"
        }
    }
}

fn ident_can_begin_expr(ident: ast::Ident) -> bool {
    let ident_token: Token = Ident(ident);
    
    !ident_token.is_any_keyword() ||
    ident_token.is_path_segment_keyword() ||
    [
        keywords::Do.name(),
        keywords::Box.name(),
        keywords::Break.name(),
        keywords::Continue.name(),
        keywords::False.name(),
        keywords::For.name(),
        keywords::If.name(),
        keywords::Loop.name(),
        keywords::Match.name(),
        keywords::Move.name(),
        keywords::Return.name(),
        keywords::True.name(),
        keywords::Unsafe.name(),
        keywords::While.name(),
    ].contains(&ident.name)
}

fn ident_can_begin_type(ident: ast::Ident) -> bool {
    let ident_token: Token = Ident(ident);
    
    !ident_token.is_any_keyword() ||
    ident_token.is_path_segment_keyword() ||
    [
        keywords::For.name(),
        keywords::Impl.name(),
        keywords::Fn.name(),
        keywords::Unsafe.name(),
        keywords::Extern.name(),
        keywords::Typeof.name(),
    ].contains(&ident.name)
}

#[derive(Clone, RustcEncodable, RustcDecodable, PartialEq, Eq, Hash, Debug)]
pub enum Token {
    /* Expression-operator symbols. */
    Eq,
    Lt,
    Le,
    EqEq,
    Ne,
    Ge,
    Gt,
    AndAnd,
    OrOr,
    Not,
    Tilde,
    BinOp(BinOpToken),
    BinOpEq(BinOpToken),
    
    /* Structural symbols */
    At,
    Dot,
    DotDot,
    DotDotDot,
    Comma,
    Semi,
    Colon,
    ModSep,
    RArrow,
    LArrow,
    FatArrow,
    Pound,
    Dollar,
    Question,
    /// An opening delimiter, eg. `{`
    OpenDelim(DelimToken),
    /// A closing delimiter, eg. `}`
    CloseDelim(DelimToken),
    
    /* Literals */
    Literal(Lit, Option<ast::Name>),

    /* Name components */
    Ident(ast::Ident),
    Underscore,
    Lifetime(ast::Ident),
    
    /* For interpolation */
    Interpolated(Rc<Nonterminal>),
    // Can be expanded into several tokens.
    /// Doc comment
    DocComment(ast::Name),
    // In right-hand-sides of MBE macros:
    /// A syntactic variable that will be filled in by macro expansion.
    SubstNt(ast::Ident),
    
    // Junk. These carry no data because we don't really care about the data
    // they *would* carry, and don't really want to allocate a new ident for
    // them. Instead, users could extract that from the associated span.
    
    /// Whitespace
    Whitespace,
    /// Comment
    Comment,
    Shebang(ast::Name),
    
    Eof,
}

impl Token {
/// Returns `true` if the token starts with '>'.
pub fn is_like_gt(&self) -> bool {
    match *self {
        BinOp(Shr) | BinOpEq(Shr) | Gt | Ge => true,
        _ => false,
    }
}

/// Returns `true` if the token can appear at the start of an expression.
pub fn can_begin_expr(&self) -> bool {
    match *self {
        Ident(ident)                => ident_can_begin_expr(ident), // value name or keyword
        OpenDelim(..)               | // tuple, array or block
        Literal(..)                 | // literal
        Not                         | // operator not
        BinOp(Minus)                | // unary minus
        BinOp(Star)                 | // dereference
        BinOp(Or) | OrOr            | // closure
        BinOp(And)                  | // reference
        AndAnd                      | // double reference
        DotDot | DotDotDot          | // range notation
        Lt | BinOp(Shl)             | // associated path
        ModSep                      | // global path
        Pound                       => true, // expression attributes
        Interpolated(ref nt) => match **nt {
            NtIdent(..) | NtExpr(..) | NtBlock(..) | NtPath(..) => true,
            _ => false,
        },
        _ => false,
    }
}

/// Returns `true` if the token can appear at the start of a type.
pub fn can_begin_type(&self) -> bool {
    match *self {
        Ident(ident)                => ident_can_begin_type(ident), // type name or keyword
        OpenDelim(Paren)            | // tuple
        OpenDelim(Bracket)          | // array
        Underscore                  | // placeholder
        Not                         | // never
        BinOp(Star)                 | // raw pointer
        BinOp(And)                  | // reference
        AndAnd                      | // double reference
        Question                    | // maybe bound in trait object
        Lifetime(..)                | // lifetime bound in trait object
        Lt | BinOp(Shl)             | // associated path
        ModSep                      => true, // global path
        Interpolated(ref nt) => match **nt {
            NtIdent(..) | NtTy(..) | NtPath(..) => true,
            _ => false,
        },
        _ => false,
    }
}

/// Returns `true` if the token is any literal
pub fn is_lit(&self) -> bool {
    match *self {
        Literal(..) => true,
            _           => false,
        }
    }
    
    pub fn ident(&self) -> Option<ast::Ident> {
        match *self {
            Ident(ident) => Some(ident),
            Interpolated(ref nt) => match **nt {
                NtIdent(ident) => Some(ident.node),
                _ => None,
            },
            _ => None,
        }
    }
    
    /// Returns `true` if the token is an identifier.
    pub fn is_ident(&self) -> bool {
        self.ident().is_some()
    }
    
    /// Returns `true` if the token is a documentation comment.
    pub fn is_doc_comment(&self) -> bool {
        match *self {
            DocComment(..)   => true,
            _                => false,
        }
    }
    
    /// Returns `true` if the token is interpolated.
    pub fn is_interpolated(&self) -> bool {
        match *self {
            Interpolated(..) => true,
            _                => false,
        }
    }
    
    /// Returns `true` if the token is an interpolated path.
    pub fn is_path(&self) -> bool {
        if let Interpolated(ref nt) = *self {
            if let NtPath(..) = **nt {
                return true;
            }
        }
        false
    }
    
    /// Returns `true` if the token is a lifetime.
    pub fn is_lifetime(&self) -> bool {
        match *self {
            Lifetime(..) => true,
                _            => false,
            }
        }
        
        /// Returns `true` if the token is either the `mut` or `const` keyword.
        pub fn is_mutability(&self) -> bool {
            self.is_keyword(keywords::Mut) ||
            self.is_keyword(keywords::Const)
        }
        
        pub fn is_qpath_start(&self) -> bool {
            self == &Lt || self == &BinOp(Shl)
        }
        
        pub fn is_path_start(&self) -> bool {
            self == &ModSep || self.is_qpath_start() || self.is_path() ||
            self.is_path_segment_keyword() || self.is_ident() && !self.is_any_keyword()
        }
        
        /// Returns `true` if the token is a given keyword, `kw`.
        pub fn is_keyword(&self, kw: keywords::Keyword) -> bool {
            self.ident().map(|ident| ident.name == kw.name()).unwrap_or(false)
        }
        
        pub fn is_path_segment_keyword(&self) -> bool {
            match self.ident() {
                Some(id) => id.name == keywords::Super.name() ||
                id.name == keywords::SelfValue.name() ||
                id.name == keywords::SelfType.name(),
                None => false,
            }
        }
        
        /// Returns `true` if the token is either a strict or reserved keyword.
        pub fn is_any_keyword(&self) -> bool {
            self.is_strict_keyword() || self.is_reserved_keyword()
        }
        
        /// Returns `true` if the token is a strict keyword.
        pub fn is_strict_keyword(&self) -> bool {
            match self.ident() {
                Some(id) => id.name >= keywords::As.name() && id.name <= keywords::While.name(),
                _ => false,
            }
        }
        
        /// Returns `true` if the token is a keyword reserved for possible future use.
        pub fn is_reserved_keyword(&self) -> bool {
            match self.ident() {
                Some(id) => id.name >= keywords::Abstract.name() && id.name <= keywords::Yield.name(),
                _ => false,
            }
        }
    }
    
    #[derive(Clone, RustcEncodable, RustcDecodable, PartialEq, Eq, Hash)]
    /// For interpolation during macro expansion.
    pub enum Nonterminal {
        NtItem(P<ast::Item>),
        NtBlock(P<ast::Block>),
        NtStmt(ast::Stmt),
        NtPat(P<ast::Pat>),
        NtExpr(P<ast::Expr>),
        NtTy(P<ast::Ty>),
        NtIdent(ast::SpannedIdent),
        /// Stuff inside brackets for attributes
        NtMeta(ast::MetaItem),
        NtPath(ast::Path),
        NtVis(ast::Visibility),
        NtTT(TokenTree),
        // These are not exposed to macros, but are used by quasiquote.
        NtArm(ast::Arm),
        NtImplItem(ast::ImplItem),
        NtTraitItem(ast::TraitItem),
        NtGenerics(ast::Generics),
        NtWhereClause(ast::WhereClause),
        NtArg(ast::Arg),
    }
    
    impl fmt::Debug for Nonterminal {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            match *self {
                NtItem(..) => f.pad("NtItem(..)"),
                NtBlock(..) => f.pad("NtBlock(..)"),
                NtStmt(..) => f.pad("NtStmt(..)"),
                NtPat(..) => f.pad("NtPat(..)"),
                NtExpr(..) => f.pad("NtExpr(..)"),
                NtTy(..) => f.pad("NtTy(..)"),
                NtIdent(..) => f.pad("NtIdent(..)"),
                NtMeta(..) => f.pad("NtMeta(..)"),
                NtPath(..) => f.pad("NtPath(..)"),
                NtTT(..) => f.pad("NtTT(..)"),
                NtArm(..) => f.pad("NtArm(..)"),
                NtImplItem(..) => f.pad("NtImplItem(..)"),
                NtTraitItem(..) => f.pad("NtTraitItem(..)"),
                NtGenerics(..) => f.pad("NtGenerics(..)"),
                NtWhereClause(..) => f.pad("NtWhereClause(..)"),
                NtArg(..) => f.pad("NtArg(..)"),
                NtVis(..) => f.pad("NtVis(..)"),
            }
        }
    }
    
    