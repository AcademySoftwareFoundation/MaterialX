# Contributing to MaterialX

Thank you for your interest in contributing to MaterialX! This document
explains our contribution process and procedures.

## Community and Discussion

There are two primary ways to connect with the MaterialX community:

- The MaterialX channel of the
[Academy Software Foundation Slack](http://academysoftwarefdn.slack.com/).
This platform is appropriate for general questions, feature requests, and
discussion of the MaterialX project as a whole.
You can request an invitation to join the Academy Software Foundation Slack
at https://www.aswf.io/get-involved/.
- The [Issues](https://github.com/AcademySoftwareFoundation/MaterialX/issues)
panel of the MaterialX GitHub, which is used both to track bugs and to discuss
feature requests.

### How to Ask for Help

If you have trouble installing, building, or using the library, but there's
not yet reason to suspect you've encountered a genuine bug, start by posting
a question to the MaterialX channel of the
[Academy Software Foundation Slack](http://academysoftwarefdn.slack.com/).
This is the place for questions such has "How do I...".

### How to Report a Bug

MaterialX uses
[GitHub Issues](https://github.com/AcademySoftwareFoundation/MaterialX/issues)
for reporting and tracking bugs, enhancements, and feature requests.

If you are submitting a bug report, please be sure to note which version of
MaterialX you are using, on what platform (OS/version, which compiler you used,
and any special build flags or other unusual environmental issues). Please
give a specific account of the following, with enough detail that others can
reproduce the problem:

- What you tried.
- What happened.
- What you expected to happen instead.

### How to Report a Security Vulnerability

If you think you've found a potential vulnerability in MaterialX, please refer
to [SECURITY.md](SECURITY.md) to responsibly disclose it.

### How to Contribute a Bug Fix or Change

To contribute code to the project, you'll need:

- Basic knowledge of Git.
- A fork of the MaterialX repository on GitHub.
- An understanding of the project's development workflow.
- Legal authorization, that is, you need to have signed a contributor
  License Agreement. See below for details.

## Legal Requirements

MaterialX is a project of the Academy Software Foundation and follows the
open source software best practice policies of the Linux Foundation.

### License

MaterialX is licensed under the [Apache 2.0](LICENSE.md) license.
Contributions to the project should abide by that standard license.

### Contributor License Agreements

To contribute to MaterialX, you must sign a Contributor License Agreement
through the *EasyCLA* system, which is integrated with GitHub as a pull
request check.

Prior to submitting a pull request, you can sign the form through
[this link](https://organization.lfx.linuxfoundation.org/foundation/a09410000182dD2AAI/project/a092M00001KWrdoQAD/cla).
If you submit a pull request before the form is signed, the EasyCLA check
will fail with a red *NOT COVERED* message, and you'll have another
opportunity to sign the form through the provided link.

- If you are an individual writing the code on your own time and you're sure
you are the sole owner of any intellectual property you contribute, you can
sign the CLA as an Individual Contributor.
- If you are writing the code as part of your job, or if your employer
retains ownership to intellectual property you create, then your company's
legal affairs representatives should sign a Corporate Contributor License
Agreement.  If your company already has a signed CCLA on file, ask your
local CLA manager to add you to your company's approved list.

The MaterialX CLAs are the standard forms used by Linux Foundation projects
and [recommended by the ASWF TAC](https://github.com/AcademySoftwareFoundation/tac/blob/main/process/contributing.md#contributor-license-agreement-cla).

## Development Workflow

### Git Basics

Working with MaterialX requires a basic understanding of Git and GitHub
terminology. If you’re unfamiliar with these concepts, please look at the
[GitHub Glossary](https://help.github.com/articles/github-glossary/) or
browse [GitHub Help](https://help.github.com/).

To contribute, you need a GitHub account. This is needed in order to push
changes to the upstream repository, via a pull request.

You will also need [Git](https://git-scm.com/doc) or a Git client such
as [Git Fork](https://git-fork.com/) or
[GitHub Desktop](https://desktop.github.com/download/) installed
on your local development machine.

### Repository Structure and Commit Policy

Development work in the MaterialX project is usually done directly
on the `main` branch. This branch represents the cutting edge of the
project and the majority of new work is proposed, tested, reviewed,
and merged there.

After sufficient work is done on the `main` branch and the MaterialX
leadership determines that a release is due, they will mark a release with
the current version tag and increment the development version for future
work. This basic repository structure keeps maintenance low, while remaining
simple to understand.

The `main` branch may include untested features and is not generally stable
enough for release. To retrieve a stable version of the source code, use any
of the 
[official releases](https://github.com/AcademySoftwareFoundation/MaterialX/releases)
of the project.

### Use the Fork, Luke.

In a typical workflow, you should *fork* the MaterialX repository to
your account. This creates a copy of the repository under your user
namespace and serves as the “home base” for your development branches,
from which you will submit *pull requests* to the upstream
repository to be merged.

Once your Git environment is operational, the next step is to locally
*clone* your forked MaterialX repository, and add a *remote*
pointing to the upstream MaterialX repository. These topics are
covered in the GitHub documentation
[Cloning a repository](https://help.github.com/articles/cloning-a-repository/)
and
[Configuring a remote for a fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/).

### Pull Requests

Contributions should be submitted as GitHub pull requests. See
[Creating a pull request](https://help.github.com/articles/creating-a-pull-request/)
if you're unfamiliar with this concept. 

The development cycle for a code change should follow this protocol:

1. Create a topic branch in your local repository, assigning the branch a
brief name that describes the feature or fix that you're working on.
2. Make changes, compile, and test thoroughly. Code style should match existing
style and conventions, and changes should be focused on the topic the pull
request will be addressing. Make unrelated changes in a separate topic branch
with a separate pull request.
3. Push commits to your fork.
4. Create a GitHub pull request from your topic branch.
5. Pull requests will be reviewed by project maintainers and contributors,
who may discuss, offer constructive feedback, request changes, or approve
the work.
6. Upon receiving the required number of approvals (as outlined in
[Required Approvals](#code-review-and-required-approvals)), a maintainer
may merge changes into the `main` branch.

### Code Review and Required Approvals

Modifications of the contents of the MaterialX repository are made on a
collaborative basis. Anyone with a GitHub account may propose a modification
via pull request and it will be considered by the project maintainers.

Pull requests must meet a minimum number of maintainer approvals prior to
being merged. Rather than having a hard rule for all PRs, the requirement
is based on the complexity and risk of the proposed changes, factoring in
the length of time the PR has been open to discussion. The following
guidelines outline the project's established approval rules for merging:

- Minor changes that don't modify current behaviors or are straightforward
fixes to existing features can be approved and merged by a single maintainer
of the repository.
- Moderate changes that modify current behaviors or introduce new features
should be approved by *two* maintainers before merging. Unless the change is
an emergency fix, the author should give the community at least 48 hours to
review the proposed change.
- Major new features and core design decisions should be discussed at length
in the ASWF Slack or in TSC meetings before any PR is submitted, in order to
solicit feedback, build consensus, and alert all stakeholders to be on the
lookout for the eventual PR when it appears.

### Developer Guidelines

The following guidelines represent coding standards that we strive to follow
in the MaterialX project. While not all existing code may adhere to these
standards yet, we encourage all new contributions to follow these practices,
and we welcome incremental improvements to bring existing code into alignment
with these guidelines.

#### Naming Conventions

Class names should use PascalCase, as in `NodeGraph` or `ShaderGenerator`.
Variable and function names should use camelCase starting with a lowercase
letter, as in `childName` or `getNode`. Protected and private member variables
additionally require an underscore prefix, as in `_parent` or `_childMap`.
Constants should be written in UPPER_CASE with underscores separating words,
as in `EMPTY_STRING` or `CATEGORY`. Type aliases should append appropriate
suffixes to indicate their purpose, using `Ptr` for pointers, `Vec` for
vectors, `Map` for maps, and `Set` for sets.

#### Static Constants and Class Organization

Class members should be organized in order of decreasing visibility: public,
protected, then private. Static constants should be placed at the end of their
respective visibility section. String constants should be defined in implementation
files rather than headers to avoid One Definition Rule violations. The
`EMPTY_STRING` constant should be used instead of empty string literals (`""`)
for clarity and consistency.

#### Smart Pointer Conventions

Heap-allocated objects in the public API should always use `shared_ptr` for
memory management. Type aliases should be defined for all shared pointers,
following the pattern of `ElementPtr` for `shared_ptr<Element>`. Both mutable
and const versions of these type aliases should be provided, e.g. `ElementPtr`
and `ConstElementPtr`. Raw pointers should be avoided except when representing
non-owning references within implementation details.

#### Const Correctness

Methods that do not modify an object's state should be marked as `const`.
Accessor methods should provide const versions to enable their use on const
objects. Type aliases following the pattern `ConstElementPtr` should be used to
indicate read-only access through shared pointers. Parameters that should not
be modified within a function should be declared as const.

#### Parameter Passing and Return Values

Strings and complex objects should be passed by `const&` to avoid unnecessary
copies. Shared pointers should be passed by value since they are designed to
be cheap to copy. When returning shared pointers, they should be returned by
value rather than by reference. Methods should be marked as `const` whenever
they do not modify the object's state.

#### Thread Safety

MaterialX classes support multiple concurrent readers, but not concurrent
reads and writes, following the pattern of standard C++ containers.  This
design enables efficient parallel processing in read-heavy workloads such
as shader generation and scene traversal, while keeping the implementation 
simple and avoiding the overhead of fine-grained locking.

#### Exception Handling

Exceptions should be used for exceptional conditions rather than for normal
control flow. Custom exception types should be defined by inheriting from
`Exception` to represent specific error categories. Exception messages should
be descriptive and include relevant context to aid in debugging. All exceptions
that may be thrown by a method should be documented using the `@throws` tag in
the method's documentation. When catching exceptions, specific exception types
should be caught rather than generic exceptions whenever possible.

#### Header Includes

Header includes should be written with angle brackets, with paths relative to
the root source folder (e.g. `#include <MaterialXCore/Element.h>`). This
ensures consistent include paths across the entire codebase, regardless of the
location of the referencing file.

Each implementation file should include its corresponding header file first,
so the first include in `Element.cpp` should be `Element.h`. This ensures that
the header file is self-contained and doesn't accidentally depend on includes
from other headers.

After the corresponding header, include blocks should be ordered hierarchically,
with high-level modules listed before low-level modules (e.g.
`MaterialXGenShader`, followed by `MaterialXFormat`, followed by
`MaterialXCore`). This maximizes opportunities to catch missing dependencies in
the high-level modules, which might otherwise be hidden at build time. Within
include blocks, individual includes should be ordered alphabetically, providing
a simple canonical order that is straightforward for developers to check.

In the interest of avoiding include cycles, developers are free to leverage
forward declarations of classes that are trivially referenced within another
header. In the interest of clarity and efficiency, developers are free to
leverage transitive header includes, where low-level headers that have already
been included by a high-level header do not need to be restated individually.

#### Coding Style

The coding style of the MaterialX project is defined by a
[clang-format](.clang-format) file in the repository, which is supported by
Clang versions 13 and newer.

When adding new source files to the repository, use the provided clang-format
file to automatically align the code to MaterialX conventions. When modifying
existing code, follow the surrounding formatting conventions so that new or
modified code blends in with the current code.

#### Documentation Standards

All classes and methods in the public API should be documented with Doxygen
comments. Classes should be documented with the `@class` tag, and structs with
the `@struct` tag, followed by a brief description and any detailed
documentation. Method documentation should include `@param`, `@return`, and
`@throws` tags where applicable. Related methods should be grouped together
using `/// @name GroupName` sections to improve readability. File-level
documentation should be placed immediately after the copyright header using
the `/// @file` directive.

#### Unit Tests

Each MaterialX module has a companion folder within the
[MaterialXTest](source/MaterialXTest) module, containing a set of unit tests
that validate its functionality. When contributing new code to MaterialX, make
sure to include appropriate unit tests in MaterialXTest to validate the
expected behavior of the new code.

The MaterialX test suite can be run locally via MaterialXTest before submitting
a pull request. Upon receiving a pull request, the GitHub CI process will
automatically run MaterialXTest across all platforms, and a successful result
is required before merging a change.
