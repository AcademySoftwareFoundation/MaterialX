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
[this link](https://contributor.easycla.lfx.linuxfoundation.org/#/cla/project/68fa91fe-51fe-41ac-a21d-e0a0bf688a53/user/564e571e-12d7-4857-abd4-898939accdd7).
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

### Coding Conventions

The coding style of the MaterialX project is defined by a
[clang-format](.clang-format) file in the repository, which is supported by
Clang versions 13 and newer.

When adding new source files to the repository, use the provided clang-format
file to automatically align the code to MaterialX conventions. When modifying
existing code, follow the surrounding formatting conventions so that new or
modified code blends in with the current code.

### Unit Tests

Each MaterialX module has a companion folder within the
[MaterialXTest](source/MaterialXTest) module, containing a set of unit tests
that validate its functionality. When contributing new code to MaterialX, make
sure to include appropriate unit tests in MaterialXTest to validate the
expected behavior of the new code.

The MaterialX test suite can be run locally via MaterialXTest before submitting
a pull request. Upon receiving a pull request, the GitHub CI process will
automatically run MaterialXTest across all platforms, and a successful result
is required before merging a change.
