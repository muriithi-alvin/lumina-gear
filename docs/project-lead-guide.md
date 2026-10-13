# Lumina Gear: Project Lead Guide

This guide is for the Project Lead, who manages the repository and keeps the team on track. The members' routine is in `lumina-gear-git-workflow.md`. This file covers what only the lead does.

---

## Table of Contents

1. [Your Responsibilities](#1-your-responsibilities)
2. [Team Pairs](#2-team-pairs)
3. [Create and Set Up the Repository](#3-create-and-set-up-the-repository)
4. [Protect the Branches](#4-protect-the-branches)
5. [Create the Starting Folders](#5-create-the-starting-folders)
6. [Open a Checkpoint](#6-open-a-checkpoint)
7. [Weekly Routine](#7-weekly-routine)
8. [Close a Checkpoint](#8-close-a-checkpoint)
9. [Final Project](#9-final-project)
10. [Problems and Fixes](#10-problems-and-fixes)
11. [Checklists](#11-checklists)

---

## 1. Your Responsibilities

- Create and own the GitHub repository and give members access.
- Protect `main` and the checkpoint branches.
- Create each checkpoint branch when its pair is ready to start.
- Make sure every pair knows its tasks and deadlines.
- Check progress and help when a pair is stuck.
- Merge finished checkpoints into `main` and create the tags.
- Make sure the submission matches the checkpoint requirements.

You are also a member of one pair, so you do coding work as well. Because you own `main`, keep your own work flowing through Pull Requests like everyone else.

---

## 2. Team Pairs

Fill in the names. The lead is in Pair A so you can set up the build system and framebuffer that everyone depends on.

| Pair | Partner 1 | Partner 2 | Owns |
|---|---|---|---|
| **Pair A** | ____________ (Project Lead) | ____________ | CP1, then Final: interface and testing |
| **Pair B** | ____________ | ____________ | CP2, then Final: simulation and animation |
| **Pair C** | ____________ | ____________ | CP3, then Final: graphics and lighting |

**Suggested matching (from the concept note):**

- Muriithi Alvin (interface), Kariuki Mary Wambui (drawing algorithms) and Billy John Igiraneza (animation) should be in **different pairs** so that each pair has a mix of skills.
- Pair B (CP2) should include the person strongest at animation.
- Pair C (CP3) should include the person most comfortable learning OpenGL.

### When a pair is not on its checkpoint

| Pair | During CP1 | During CP2 | During CP3 |
|---|---|---|---|
| A | **Owns CP1** | Reviews, fixes bugs in CP1 code | Reviews |
| B | Prepares for transforms and animation | **Owns CP2** | Reviews |
| C | Learns OpenGL basics early | Keeps learning OpenGL | **Owns CP3** |

Ask the waiting pairs to review Pull Requests when asked and to keep notes on what they will need.

---

## 3. Create and Set Up the Repository

### 3.1 Install and set up Git

```bash
git --version
git config --global user.name "Your Full Name"
git config --global user.email "your.email@example.com"
git config --global init.defaultBranch main
```

### 3.2 Create the repository on GitHub

1. Click **New repository**.
2. Name: `lumina-gear`.
3. Choose **Private** (or Public if your lecturer needs it).
4. Tick **Add a README file**.
5. Choose the **C++** `.gitignore` template.
6. Click **Create repository**.

### 3.3 Add the team

Go to **Settings > Collaborators > Add people** and add the other five members. Each person must accept the invitation by email.

### 3.4 Clone the repository

```bash
git clone https://github.com/<your-username>/lumina-gear.git
cd lumina-gear
git remote -v
```

### 3.5 Extend the `.gitignore`

Open `.gitignore` and add these lines if they are missing:

```
build/
out/
*.o
*.exe
.vscode/
.idea/
```

Save it:

```bash
git add .gitignore
git commit -m "chore: extend gitignore"
git push origin main
```

(This is the only direct push to `main`. Do it before branch protection is turned on.)

---

## 4. Protect the Branches

On GitHub, go to **Settings > Branches > Add branch protection rule**.

### Rule 1: `main`

- Branch name pattern: `main`
- Tick **Require a pull request before merging**
- Tick **Require approvals** and set it to **1**
- Tick **Do not allow bypassing the above settings** (this also stops accidental direct pushes)

### Rule 2: checkpoint branches

- Branch name pattern: `cp*`
- Tick **Require a pull request before merging**
- Tick **Require approvals** and set it to **1**

Also add another rule for `final-project` with the same settings.

Under **Settings > General > Pull Requests**, tick **Automatically delete head branches** so merged task branches are removed for you.

---

## 5. Create the Starting Folders

Do this once, on the CP1 branch, so everyone uses the same layout.

```bash
git checkout main
git pull origin main
git checkout -b cp1-technical-foundation
mkdir -p docs/screenshots include/lumina src tests
touch docs/screenshots/.gitkeep tests/.gitkeep
```

Layout to follow:

```
lumina-gear/
├── README.md
├── CMakeLists.txt
├── docs/
├── include/lumina/    (framebuffer.h, draw.h, components.h, simulation.h, ui.h, effects.h)
├── src/               (matching .cpp files and main.cpp)
└── tests/
```

Commit and upload:

```bash
git add .
git commit -m "chore: add initial project folder structure"
git push -u origin cp1-technical-foundation
```

Because the CP1 branch was created with a commit on it, you now push it to GitHub. Protection rule 2 applies from this point, so the folders go in before the rule matters (or set the rule after this push).

---

## 6. Open a Checkpoint

CP1 was created in Section 5. For every later checkpoint, follow these steps when the checkpoint's pair is ready.

### Step 1: Start from the latest `main`

```bash
git checkout main
git pull origin main
```

### Step 2: Create and upload the checkpoint branch

```bash
git checkout -b cp2-transformation-animation
git push -u origin cp2-transformation-animation
```

Use these names:

| Checkpoint | Branch |
|---|---|
| CP2 | `cp2-transformation-animation` |
| CP3 | `cp3-opengl-prototype` |
| Final | `final-project` |

### Step 3: Tell the pair

Send a short message: the branch name, the task list for that checkpoint (from the member guide), the deadline, and the person who reviews.

### Step 4: Members fetch the branch

They run:

```bash
git fetch origin
git checkout cp2-transformation-animation
git pull origin cp2-transformation-animation
```

---

## 7. Weekly Routine

Keep it short and regular.

**Check progress**

```bash
git fetch origin
git branch -r
git log origin/cp1-technical-foundation --oneline --graph -15
```

On GitHub, look at **Pull requests** for open ones waiting for review and nudge people if a review has waited more than a day.

**Check that the checkpoint branch builds**

```bash
git checkout cp1-technical-foundation
git pull origin cp1-technical-foundation
mkdir -p build && cd build
cmake ..
cmake --build .
cd ..
```

If the build fails, find who broke it:

```bash
git log --oneline -10
```

and ask that person to fix it with a `fix/` branch.

**Weekly team check (10 minutes)**

1. What did each pair finish?
2. What is blocking anyone?
3. What is the plan for next week?

---

## 8. Close a Checkpoint

Do this when all task branches for the checkpoint are merged.

### Step 1: Final test

```bash
git checkout cp1-technical-foundation
git pull origin cp1-technical-foundation
mkdir -p build && cd build
cmake ..
cmake --build .
cd ..
```

Run the program and go through the [checkpoint checklist](#11-checklists). Any problems found are fixed through normal `fix/` branches and Pull Requests first.

### Step 2: Open a Pull Request into `main`

On GitHub:

- **Base:** `main`
- **Compare:** `cp1-technical-foundation`
- **Title:** `CP1: Technical Foundation`
- **Description:** what is included and how to run it
- **Reviewer:** a member of the pair that owned the checkpoint

### Step 3: Merge

After approval, click **Merge pull request** (not squash, so the history stays).

### Step 4: Tag the version

```bash
git checkout main
git pull origin main
git tag -a v1.0-cp1 -m "CP1: Technical Foundation submission"
git push origin v1.0-cp1
git tag
```

| Checkpoint | Tag |
|---|---|
| CP1 | `v1.0-cp1` |
| CP2 | `v2.0-cp2` |
| CP3 | `v3.0-cp3` |
| Final | `v4.0-final` |

### Step 5: Confirm what you are submitting

```bash
git checkout v1.0-cp1
```

Build and run it one more time from the tag. Then go back:

```bash
git checkout main
```

Submit the repository link and tag name, or whatever your lecturer asks for.

### Step 6: Prepare the next checkpoint

Open the next checkpoint branch from the updated `main` (Section 6). This way the next pair starts with all earlier work.

### Step 7: Fix problems found later

If a bug in an earlier checkpoint is found during a later one, fix it on the **current** checkpoint branch using a `fix/` branch. Do not edit old tags.

---

## 9. Final Project

Three pairs work at the same time, each with its own area:

| Pair | Branch | Area |
|---|---|---|
| Pair A | `feature/final-ui-testing` | Interface polish, full testing, bug list |
| Pair B | `feature/final-simulation-animation` | Efficiency rules, edge cases, smooth animation |
| Pair C | `feature/final-graphics-lighting` | Visual polish, lighting, demo video |

Steps:

1. Create `final-project` from `main` after the CP3 tag (Section 6).
2. Each pair makes task branches from `final-project` and merges back by Pull Request.
3. Because three pairs now edit the same project, remind everyone to pull daily and stay in their own files.
4. Hold a **feature freeze** a few days before the deadline. After the freeze, only `fix/` branches are allowed.
5. Close the checkpoint as in Section 8 and tag `v4.0-final`.

---

## 10. Problems and Fixes

**A member pushed to the wrong branch**

If it is on a task branch, nothing to fix. If it was committed to a checkpoint branch by mistake, undo it safely:

```bash
git checkout cp1-technical-foundation
git pull origin cp1-technical-foundation
git revert <commit-hash>
git push origin cp1-technical-foundation
```

(If protection is on, do this through a Pull Request from a `fix/` branch.)

**A merge conflict is too hard for the pair**

```bash
git fetch origin
git checkout feature/cp1-some-task
git merge origin/cp1-technical-foundation
```

Open the conflicted files with the pair, decide together, then:

```bash
git add <files>
git commit -m "merge: resolve conflict"
git push
```

**Someone's branch is far behind**

Ask them to run:

```bash
git fetch origin
git merge origin/<checkpoint-branch>
```

**Build files were committed by mistake**

```bash
git rm -r --cached build/
git commit -m "chore: remove build files from tracking"
git push
```

**A tag was created on the wrong commit (before submission)**

```bash
git tag -d v1.0-cp1
git push origin --delete v1.0-cp1
git checkout main
git tag -a v1.0-cp1 -m "CP1: Technical Foundation submission"
git push origin v1.0-cp1
```

**See who changed a line**

```bash
git blame src/draw.cpp
```

**A member cannot push**

Check, in this order: they accepted the GitHub invitation; they are pushing to a task branch, not a protected one; they are logged in to GitHub on their computer.

---

## 11. Checklists

### Before opening a checkpoint

- [ ] Previous checkpoint is merged into `main` and tagged
- [ ] Branch created from the latest `main`
- [ ] Pair knows its tasks, deadline and reviewer
- [ ] Other pairs know their support role

### Before closing a checkpoint

- [ ] All task branches merged, none left open
- [ ] Project builds on a clean clone
- [ ] All checkpoint requirements demonstrated (see the checkpoint descriptions)
- [ ] README explains how to build and run
- [ ] Screenshots or demo added to `docs/`
- [ ] Pull Request into `main` approved
- [ ] Tag created and uploaded

### Test on a clean clone (do this before every submission)

```bash
cd ..
git clone https://github.com/<your-username>/lumina-gear.git lumina-gear-test
cd lumina-gear-test
git checkout v1.0-cp1
mkdir build && cd build
cmake ..
cmake --build .
```

If this works, anyone can build your submission.

### Checkpoint requirements (from the course)

| Checkpoint | Requirement |
|---|---|
| CP1 | Implement the graphics techniques covered so far in pure C++ and build simplified project components |
| CP2 | Add transformations and animation to the CP1 components in pure C++ |
| CP3 | Integrate the components and earlier work into OpenGL and build a working prototype |
| Final | Refine and complete the project with the required functionality, graphics, interaction and animation |
