# Ruby 0.95

**Ruby 0.95** was the first publicly released version of Ruby. It was originally published on December 21, 1995.

This is an attempt to get Ruby 0.95 to compile on a modern version of Linux (32-bit Ubuntu 16.04) using a modern compiler (GCC 5.4).

## Why?

- **Why do it?** Mostly because I thought it would make for a fun talk at a Ruby meet-up or conference.
- **Why Ubuntu 16.04?** It's the last version of Ubuntu for which [32-bit images were published](https://lists.ubuntu.com/archives/ubuntu-release/2017-September/004212.html)
- **Why 32-bit?** 64-bit desktop systems weren't really a thing back in 1995: the first x86-64/AMD64 CPU was released in 2003.
- **Why Linux in the first place?** My normal development computer is a Mac, but macOS/OS X wouldn't be released until 2001.

## Compiling

This has only been tested with **Ubuntu 16.04** (with GCC 5.4), but other distros would probably work too.

1. Install the build dependencies

```sh
sudo apt install autoconf bison build-essential flex git
```

2. Clone this repo

```sh
git clone https://github.com/matiaskorhonen/ruby-0.95.git
cd ruby-0.95
```

3. Make the destination folders (I'm assuming that you don't want to install this system-wide)

```sh
# Note that this is a different directory than the git repo
mkdir -p ~/opt/bin
mkdir -p ~/opt/lib/ruby
```

4. Configure the build

```sh
./configure --host=i386-unknown-linux --prefix="$HOME/opt"
```

5. Compile the code

```sh
make
```

6. Installed the newly compiled Ruby 0.95

```sh
make install
```

7. Enjoy your ancient Ruby

```sh
echo 'print "Hello from Ruby 0.95\n"' | $HOME/opt/bin/ruby
```

## Docker support

There's rudimentary Docker support.

```sh
docker build --tag=ruby95 .

# Run a Ruby 0.95 program from STDIN
echo "print 'Hello world\n'" | docker run --interactive ruby95

# Run one of the samples included in the Docker image
docker run ruby95 sample/dir.rb
```

## Caveats

- It segfaults if you look at it wrong
- Many things that you might be used to don't exist (e.g. `puts` or anything HTTP related)
- None of the extensions from `ext/` are included because I could get the setup script to stop segfaulting

## Help wanted

- **Segfaults:** pull requests or tips about fixing the segfaults would be highly appreciated
- **Autoconf:** updating the autoconf setup would probably make it a fair bit easier to try out Ruby 0.95. Again, pull requests would be welcomed…
