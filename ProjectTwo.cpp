#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

// CS 300 Project Two - ABCU Advising Assistance Program
// AI Use Acknowledgment: I used ChatGPT, developed by OpenAI, to help organize,
// revise, and test portions of this code. I reviewed the final program and
// verified that it addresses the assignment requirements and course materials.

struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

struct TreeNode {
    Course course;
    TreeNode* left;
    TreeNode* right;

    explicit TreeNode(const Course& courseData)
        : course(courseData), left(nullptr), right(nullptr) {}
};

class CourseTree {
private:
    TreeNode* root;

    static void destroyTree(TreeNode* node) {
        if (node == nullptr) {
            return;
        }
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

    static TreeNode* insertNode(TreeNode* node, const Course& course) {
        if (node == nullptr) {
            return new TreeNode(course);
        }

        if (course.courseNumber < node->course.courseNumber) {
            node->left = insertNode(node->left, course);
        } else if (course.courseNumber > node->course.courseNumber) {
            node->right = insertNode(node->right, course);
        } else {
            // Replace an existing course if the same course number is inserted.
            node->course = course;
        }

        return node;
    }

    static void printInOrder(TreeNode* node) {
        if (node == nullptr) {
            return;
        }

        printInOrder(node->left);
        cout << node->course.courseNumber << ", " << node->course.courseTitle << '\n';
        printInOrder(node->right);
    }

public:
    CourseTree() : root(nullptr) {}

    ~CourseTree() {
        clear();
    }

    CourseTree(const CourseTree&) = delete;
    CourseTree& operator=(const CourseTree&) = delete;

    void clear() {
        destroyTree(root);
        root = nullptr;
    }

    void insert(const Course& course) {
        root = insertNode(root, course);
    }

    const Course* search(const string& courseNumber) const {
        TreeNode* current = root;

        while (current != nullptr) {
            if (courseNumber == current->course.courseNumber) {
                return &current->course;
            }

            if (courseNumber < current->course.courseNumber) {
                current = current->left;
            } else {
                current = current->right;
            }
        }

        return nullptr;
    }

    void printCourseList() const {
        printInOrder(root);
    }
};

// Removes whitespace at the beginning and end of a CSV field.
string trim(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

// Converts course numbers entered by the user to uppercase so values such as
// "csci400" match the stored course number "CSCI400".
string toUpperCase(string value) {
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return value;
}

// Splits one comma-separated line into individual fields.
vector<string> splitCsvLine(const string& line) {
    vector<string> fields;
    string field;
    stringstream stream(line);

    while (getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }

    // Preserve an empty final field when a line ends with a comma.
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }

    return fields;
}

// Reads and validates the file before replacing the courses currently stored
// in the binary search tree.
bool loadCourses(const string& fileName, CourseTree& courses) {
    ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        cout << "Error: Could not open file " << fileName << "." << '\n';
        return false;
    }

    vector<Course> parsedCourses;
    unordered_set<string> courseNumbers;
    string line;
    int lineNumber = 0;

    while (getline(inputFile, line)) {
        ++lineNumber;

        // Remove a carriage return if the file uses Windows line endings.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (trim(line).empty()) {
            continue;
        }

        vector<string> fields = splitCsvLine(line);

        if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) {
            cout << "Error: Invalid course data on line " << lineNumber
                 << ". Each course must include a course number and title." << '\n';
            return false;
        }

        Course course;
        course.courseNumber = toUpperCase(fields[0]);
        course.courseTitle = fields[1];

        // Ignore empty prerequisite fields, which are valid for courses that
        // have no prerequisites.
        for (size_t i = 2; i < fields.size(); ++i) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(toUpperCase(fields[i]));
            }
        }

        if (courseNumbers.find(course.courseNumber) != courseNumbers.end()) {
            cout << "Error: Duplicate course number " << course.courseNumber
                 << " found in the data file." << '\n';
            return false;
        }

        courseNumbers.insert(course.courseNumber);
        parsedCourses.push_back(course);
    }

    inputFile.close();

    if (parsedCourses.empty()) {
        cout << "Error: The course data file is empty." << '\n';
        return false;
    }

    // Validate prerequisites only after the entire file has been read because
    // a prerequisite may appear later in the input file.
    for (const Course& course : parsedCourses) {
        for (const string& prerequisite : course.prerequisites) {
            if (courseNumbers.find(prerequisite) == courseNumbers.end()) {
                cout << "Error: Prerequisite " << prerequisite
                     << " for " << course.courseNumber
                     << " does not exist in the course data file." << '\n';
                return false;
            }
        }
    }

    courses.clear();
    for (const Course& course : parsedCourses) {
        courses.insert(course);
    }

    cout << "Course data loaded successfully." << '\n';
    return true;
}

void displayMenu() {
    cout << "1. Load Data Structure." << '\n';
    cout << "2. Print Course List." << '\n';
    cout << "3. Print Course." << '\n';
    cout << "9. Exit" << '\n';
    cout << "What would you like to do? ";
}

void printCourseInformation(const CourseTree& courses, const string& userInput) {
    const string courseNumber = toUpperCase(trim(userInput));
    const Course* course = courses.search(courseNumber);

    if (course == nullptr) {
        cout << courseNumber << " is not a valid course." << '\n';
        return;
    }

    cout << course->courseNumber << ", " << course->courseTitle << '\n';

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None" << '\n';
        return;
    }

    cout << "Prerequisites: ";

    for (size_t i = 0; i < course->prerequisites.size(); ++i) {
        const Course* prerequisite = courses.search(course->prerequisites[i]);

        if (i > 0) {
            cout << "; ";
        }

        if (prerequisite != nullptr) {
            // The rubric requires both prerequisite numbers and titles.
            cout << prerequisite->courseNumber << ", " << prerequisite->courseTitle;
        } else {
            // This should not occur because prerequisites are validated at load time.
            cout << course->prerequisites[i];
        }
    }

    cout << '\n';
}

int main() {
    CourseTree courses;
    bool dataLoaded = false;
    string choice;

    cout << "Welcome to the course planner." << '\n';

    while (true) {
        displayMenu();
        getline(cin, choice);
        choice = trim(choice);

        if (choice == "1") {
            string fileName;
            cout << "What is the name of the course data file? ";
            getline(cin, fileName);
            fileName = trim(fileName);

            dataLoaded = loadCourses(fileName, courses);
        } else if (choice == "2") {
            if (!dataLoaded) {
                cout << "Please load the course data first." << '\n';
            } else {
                cout << "Here is a sample schedule:" << '\n';
                courses.printCourseList();
            }
        } else if (choice == "3") {
            if (!dataLoaded) {
                cout << "Please load the course data first." << '\n';
            } else {
                string courseNumber;
                cout << "What course do you want to know about? ";
                getline(cin, courseNumber);
                printCourseInformation(courses, courseNumber);
            }
        } else if (choice == "9") {
            cout << "Thank you for using the course planner!" << '\n';
            break;
        } else {
            cout << choice << " is not a valid option." << '\n';
        }
    }

    return 0;
}
