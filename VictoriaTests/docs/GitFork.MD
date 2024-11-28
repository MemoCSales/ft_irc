If you want to fork a collaboration repository to show its content in your own GitHub account, you can follow these steps:

1. **Fork the Repository:**
	- Go to the repository you want to fork on GitHub.
	- Click the `Fork` button in the top right corner of the repository page.
	- Select your GitHub account as the destination for the fork.

2. **Clone the Forked Repository Locally:**
	```bash
	git clone https://github.com/your-username/forked-repository.git
	cd forked-repository
	```

3. **Set Up the Upstream Remote:**
	- This allows you to pull in changes from the original repository.
	```bash
	git remote add upstream https://github.com/VictoriaLizCor/ft_irc.git
	```

4. **Fetch and Merge Changes from Upstream:**
	- Periodically, you should fetch and merge changes from the original repository to keep your fork up to date.
	```bash
	git fetch upstream
	git checkout main
	git merge upstream/main
	```

5. **Push Changes to Your Fork:**
	- If you make changes to your fork and want to push them to your GitHub account:
	```bash
	git add .
	git commit -m "Your commit message"
	git push origin main
	```

6. **Create Pull Requests:**
	- If you want to contribute back to the original repository, create a pull request from your fork.
	- Go to your forked repository on GitHub.
	- Click on `New pull request`.
	- Select the branches and repositories to merge from and into.
	- Provide a description of your changes and submit the pull request.

Here’s a summary of the commands:

```bash
# Clone your forked repository
git clone https://github.com/your-username/forked-repository.git
cd forked-repository

# Add the original repository as an upstream remote
git remote add upstream https://github.com/VictoriaLizCor/ft_irc.git
# Fetch and merge changes from the upstream repository
git fetch upstream
git checkout main
git merge upstream/main

# Push changes to your fork
git add .
git commit -m "Your commit message"
git push origin main
```

By following these steps, you can fork a collaboration repository to your own GitHub account, keep it up to date with the original repository, and contribute back to the original project.