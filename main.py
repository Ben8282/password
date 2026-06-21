import os
import difflib
print("Welcome to our Password Manager Version 1")
print("Would you like to add a password or view one?")

reset = 1

while reset == 1:
    print("Type 1 to add a password")
    print("Type 2 to view a password")

    response = input("> ")

    if response == "1":
        if os.path.exists("password.txt"):
            print("please input the name of the password you would like to put")
            name = input()
            print("please input the password you would like to add that will be saved with", name)
            password = input()
            next_id = 1
            with open("password.txt", "r") as rf:
                for line in rf:
                    parts = line.strip().split("|")
                    id_num = int(parts[0])
                    next_id = id_num + 1
            with open("password.txt", "a") as f:
                entry = f"{next_id}|{name}|{password}\n"
                print(entry)  # remove this later
                f.write(entry)
            print("Adding a password...")
            print("would you like to add or view another password")
            print("if yes say 1 if no say 2 or another number")
            resetq = input()
            if resetq == "1":
                reset = 1
            else:
                reset = 0
        else:
            print("please input the name of the password you would like to put")
            name = input()
            print("please input the password you would like to add that will be saved with the name", name)
            password = input()
            next_id = 1
            with open("password.txt", "w") as f:
                entry = f"{next_id}|{name}|{password}\n"
                print(entry)  # remove this later
                f.write(entry)
            print("Adding a password...")
            print("would you like to add or view another password")
            print("if yes say 1 if no say 2 or another number")
            resetq = input()
            if resetq == "1":
                reset = 1
            else:
                reset = 0

    elif response == "2":
        if not os.path.exists("password.txt"):
            print("No passwords saved yet. Add one first.")
        else:
            go_again = 1
            while go_again == 1:
                print("what would you like to view type 1 if you would like to view a specific password or 2 if you would like to view all your passwords")
                awsr = input()
                if awsr == "1":
                    a = 1
                    while a == 1:
                        print("would you like to input a number that is in order of which password you added or search for name of the password")
                        print("press 1 for search by order of adding (search by id) or 2 for name")
                        password_or_name = input()
                        if password_or_name == "1":
                            print("enter ID")
                            searchpassword = input()
                            with open("password.txt", "r") as rf:
                                found = False
                                for line in rf:
                                    parts = line.strip().split("|")
                                    if parts[0] == searchpassword:
                                        print(f"Name: {parts[1]}")
                                        print(f"Password: {parts[2]}")
                                        found = True
                                        break
                                if not found:
                                    print("No password found with that ID.")
                            a = 0
                        elif password_or_name == "2":
                            print("enter name to search")
                            search_name = input().lower()
                            with open("password.txt", "r") as rf:
                                entries = []
                                for line in rf:
                                    parts = line.strip().split("|")
                                    stored_name = parts[1].lower()
                                    ratio = difflib.SequenceMatcher(None, search_name, stored_name).ratio()
                                    if search_name in stored_name or ratio >= 0.6:
                                        entries.append((ratio, parts[1], parts[2]))
                                entries.sort(reverse=True)
                                if entries:
                                    print(f"Found {len(entries)} match(es):")
                                    for _, entry_name, entry_password in entries:
                                        print(f"  Name: {entry_name}  |  Password: {entry_password}")
                                else:
                                    print("No matches found.")
                            a = 0
                        else:
                            a = 1
                    go_again = 0
                elif awsr == "2":
                    with open("password.txt", "r") as rf:
                        lines = rf.readlines()
                    if not lines:
                        print("No passwords saved yet.")
                    else:
                        print(f"\n--- All Passwords ({len(lines)} total) ---")
                        for line in lines:
                            parts = line.strip().split("|")
                            print(f"  ID: {parts[0]}  |  Name: {parts[1]}  |  Password: {parts[2]}")
                        print("-----------------------------------\n")
                    go_again = 0
                else:
                    print("invalid answer")
            print("Viewing a password...")
        reset = 0

    else:
        print("Please enter either 1 or 2.")
