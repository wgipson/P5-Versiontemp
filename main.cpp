//Project UID db1f506d06d84ab787baf250c265e24e
//why can I not open csvstream.v?
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include "csvstream.h"

using namespace std;

set<string> unique_words(const string& str);

class classifier
{

public:
    //EFFECT: Print classifier info
    void info()
    {
        cout << "vocabulary size = " << vocab_size << endl
            << endl;
        cout << "classes:" << endl;
        for (auto& key : num_posts_with_C)
        {
            cout << "  " << key.first << ", " << key.second << " examples, log-prior = "
                << compute_log_prior(key.first) << endl;
        }

        cout << "classifier parameters:" << endl;
        for (auto& k_v : num_posts_C_w)
        {
            cout << "  " << k_v.first.first << ":" << k_v.first.second << ", count = "
                << k_v.second << ", log-likelihood = "
                << compute_log_likelihood(k_v.first.first, k_v.first.second) << endl;
        }
        cout << endl;
    }

    //EFFECT: Train the classifier with the training set provided
    void train(ifstream& in, int argc)
    {
        csvstream in_train(in);
        map<string, string> temp;
        set<string> unique_set;

        //Print training data:
        if (argc == 4)
            cout << "training data:" << endl;

        while (in_train >> temp)
        {
            if (argc == 4)
                cout << "  label = " << temp["tag"]
                << ", content = " << temp["content"] << endl;

            set<string> unique = unique_words(temp["content"]);
            string tag = temp["tag"];

            //Count the number of posts in total
            num_posts++;

            //Count the number if unique words
            unique_set.insert(unique.begin(), unique.end());

            //For each word w, the number of posts in the entire set that contains w
            for (auto& i : unique)
                num_posts_contain_w[i]++;

            //Post with the predicted_label "tag"
            num_posts_with_C[tag]++;

            //Posts containing word w and predicted_label C
            for (auto& i : unique)
                num_posts_C_w[{tag, i}]++;
        }

        //Count the number if unique words
        vocab_size = unique_set.size();

        //The number of training posts
        cout << "trained on " << get_num_posts() << " examples" << endl;

        //Info on classifier
        if (argc == 4)
            info();
    }

    //EFFECT: Test the accuracy of the classifier with the testing set provided
    map<string, pair<string, double>> test(ifstream& in, int argc)
    {
        csvstream in_test(in);
        map<string, string> temp;
        map<string, pair<string, double>> results;
        int predicted_correctly = 0;
        int total_predicted = 0;

        cout << "test data:" << endl;

        while (in_test >> temp)
        {
            string content = temp["content"];
            double highest = 0;
            string best;
            set<string> words_in_post = unique_words(content);

            for (auto& i : num_posts_with_C)
            {
                double p = compute(i.first, words_in_post);
                if (i == *num_posts_with_C.begin())
                {
                    highest = p;
                    best = i.first;
                    continue;
                }
                if (p > highest)
                {
                    highest = p;
                    best = i.first;
                }
            }
            results[content] = { best, highest };

            string actual_label = temp["tag"];
            string predicted_label = results[content].first;
            double prob = results[content].second;

            if (predicted_label == actual_label)
                predicted_correctly++;
            total_predicted++;

            cout << "  correct = " << actual_label << ", predicted = " << predicted_label
                << ", log-probability score = " << prob << endl;
            cout << "  content = " << content << endl
                << endl;
        }
        cout << "performance: " << predicted_correctly << " / "
            << total_predicted << " posts predicted correctly" << endl;

        return results;
    }

    int get_num_posts()
    {
        return num_posts;
    }

    int get_vocab_size()
    {
        return vocab_size;
    }

    int get_num_post_with_C(string C)
    {
        return num_posts_with_C[C];
    }

    int get_num_post_contain_w(string w)
    {
        return num_posts_contain_w[w];
    }

    int get_num_post_with_C_contain_w(string C, string w)
    {
        return num_posts_C_w[{C, w}];
    }

private:
    //EFFECT: Compute the log-probability score with the given predicted_label
    double compute(string C, set<string> words_in_post)
    {
        double result = compute_log_prior(C);
        for (auto& i : words_in_post)
        {
            result += compute_log_likelihood(C, i);
        }
        return result;
    }

    //EFFECT: Compute the log prior of a given predicted_label C
    double compute_log_prior(string C)
    {

        return log(num_posts_with_C[C] / num_posts);
    }

    //EFFECT: Compute the log_likelihood of a word w given predicted_label C
    double compute_log_likelihood(string C, string w)
    {

        if (num_posts_contain_w[w] == 0)
            return log(1.0 / num_posts);
        if (num_posts_C_w[{C, w}] == 0)
            return log(num_posts_contain_w[w] / num_posts);

        return log(num_posts_C_w[{C, w}] / num_posts_with_C[C]);
    }

private:
    //Total number of training posts;
    int num_posts = 0;

    //Number of unique words in training posts
    int vocab_size = 0;

    //Number of posts containing w
    map<string, double> num_posts_contain_w;

    //Number of posts with different labels
    map<string, double> num_posts_with_C;

    //Number of posts with predicted_label C that contains w
    map<pair<string, string>, double> num_posts_C_w;
};

void summary(classifier& piazza, csvstream& in_train, csvstream& in_test, int argc);

int main(int argc, const char* argv[])
{
    cout.precision(3);

    if (argc < 3)
    {
        cout << "Usage: main.exe TRAIN_FILE TEST_FILE [--debug]" << endl;
        return 1;
    }
    if (argc == 4 && strcmp(argv[3], "--debug") != 0)
    {
        cout << "Usage: main.exe TRAIN_FILE TEST_FILE [--debug]" << endl;
        return 1;
    }

    string train_file = argv[1], test_file = argv[2];
    ifstream train(train_file), test(test_file);

    classifier piazza;

    piazza.train(train, argc);
    map<string, pair<string, double>> results = piazza.test(test, argc);

    return 0;
}

set<string> unique_words(const string& str)
{
    istringstream source(str);
    set<string> words;
    string word;

    while (source >> word)
    {
        words.insert(word);
    }

    return words;
}