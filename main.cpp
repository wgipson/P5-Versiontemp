//Project UID db1f506d06d84ab787baf250c265e24e
//why can I not open csvstream.v?
#include <iostream>
#include <fstream>
#include <set>
#include <cmath>
#include <cstdlib>
#include <vector>
#include "csvstream.h"
#include <string>
#include <cassert>
#include <map>

using namespace std;

//I need to design the entire classifier program I guess--
// Lets look at the spec for info on the 
//classification
/* 
the private data members for the class should keep track 
of the classifier parameters 
learned from the training  data, and the public member 
functions should provide an 
interface that allows you to train the classifier and make 
predictions for new piazza posts.
*/
class classifier {
private:
    int numposts = 0;
    set<string> uniquewords;
    set<string>uniquetags;
    map<string, double> uniquetags2;
    vector<string> tag;
    vector<string> post;
    vector<set<string>> vectorposts;
    map<string, int> num_posts_withC;
    map<string, int> num_posts_withW;
    map<pair<string, string>, int> num_posts_with_wc;
    map<string, string> data;

    void unique_words() {
        for (int i = 0; i < numposts; i++)
        {
            for (auto j = vectorposts[i].begin(); 
                j != vectorposts[i].end(); j++) {
                istringstream source(*j);
                set<string> words;
                string word;
                while (source >> word) {
                    uniquewords.insert(word);
                }
            }

        }
    }

    set<string> split(const string& data)
    {
        set<string> thispost;
        istringstream source(data);
        string word;
        while (source >> word)
        {
            thispost.insert(word);
        }
        return thispost;
    }
public:
    //get your numbers needed for printing and calculation
    int num_posts() {
        return numposts;
    }
    int num_unique_words() {
        return uniquewords.size();
    }
    void readin(csvstream& data_in) {
        map<string, string> temp;
        while (data_in >> temp) {
            tag.push_back(temp["tag"]);
            uniquetags.insert(temp["tag"]);
            uniquetags2.insert({ temp["tag"], 0 });
            post.push_back(temp["content"]);
            vectorposts.push_back(split(temp["content"]));
            numposts++;
        }
        unique_words();
    }
    string returnTag(const int& pos)
    {
        return tag[pos];
    }
    string returnPost(const int& pos)
    {
        return post[pos];
    }
    void count()
    {
        countUW();
        countUT();
        countWC();
    }
    void countUT() {
        for (auto it = uniquetags.begin(); it != uniquetags.end(); it++)
        {
            int count = 0;
            for (int j = 0; j < num_posts(); j++)
                if (tag[j] == *it)
                    count++;
            num_posts_withC[*it] = count;
        }
    }
    void countUW() {
        for (auto it = uniquewords.begin(); it != uniquewords.end(); it++)
        {
            int count = 0;
            for (int i = 0; i < num_posts(); i++)
            {
                if (vectorposts[i].find(*it) != vectorposts[i].end())
                    count++;
            }
            num_posts_withW[*it] = count;
        }
    }
    void countWC()
    {
    for (auto it = uniquewords.begin(); it != uniquewords.end(); it++)
        for (auto a = uniquetags.begin(); a != uniquetags.end(); a++)
        {
            int count = 0;
            for (int i = 0; i < num_posts(); i++)
            {
                if (tag[i] == *a && vectorposts[i].find(*it) 
                    != vectorposts[i].end())
                    count++;
            }
            pair<string, string> temp;
            temp.first = *a;
            temp.second = *it;
            num_posts_with_wc[temp] = count;
        }
    }
    //do your calculations and construct a finder function 
    // for the prediction of each post
    //find probability of each label for a post
    void getprobabilitieschunk(set<string>::iterator x, const int i, double &labelprob) {
        labelprob = log(num_posts_withC[*x] /
            static_cast<double>(num_posts()));
        for (auto y = vectorposts[i].begin(); y != vectorposts[i].end(); y++)
        {
            int integer = num_posts_with_wc[{*x, * y}];
            if (integer > 0) {
                labelprob += log(integer / static_cast<double>
                    (num_posts_withC[*x]));
            }
            else
            {
                integer = num_posts_withW[*y];
                if (integer > 0)
                    labelprob += log(integer /
                        static_cast<double>(num_posts()));
                else
                    labelprob += log(1 /
                        static_cast<double>(num_posts()));
            }
        }
    }

    pair<string, double> getProbabilitiesTraining(const int i)
    {
        //we could just do this for each post
            for (auto x = uniquetags.begin(); x != uniquetags.end(); x++)
            {
                double labelprob = log(num_posts_withC[*x] /
                    static_cast<double>(num_posts()));
                getprobabilitieschunk(x, i, labelprob);
                uniquetags2[*x] = labelprob;
            }
            pair<string, double> max;
            max.first = uniquetags2.begin()->first;
            max.second = uniquetags2.begin()->second;
            for (auto itr = uniquetags2.begin();
                itr != uniquetags2.end();
                ++itr)
            {
                if (itr->second > max.second) {
                    max.first = itr->first;
                    max.second = itr->second;
                }
               
            }
            return max;
            //find max in uniquetags2 and its associated label
    }
    void getProbabilitiesTest(classifier& input, vector<map<string, 
        double>>& probabilitiesfortest) {
        for (int i = 0; i < num_posts(); i++)
        {
            map<string, double> temp;
            for (auto x = input.uniquetags.begin(); x != input.uniquetags.end(); x++)
            {
                double probability = log(input.num_posts_withC[*x] /
                    static_cast<double>(input.num_posts()));
                for (auto y = vectorposts[i].begin(); y != vectorposts[i].end(); y++)
                {
                    int integer = input.num_posts_with_wc[{*x, *y}];
                    if (integer > 0) {
                        probability += log(integer / static_cast<double>
                            (input.num_posts_withC[*x]));
                    }
                    else
                    {
                        integer = input.num_posts_withW[*y];
                        if (integer > 0) {
                            probability += log(integer / 
                                static_cast<double>(input.num_posts()));
                        }
                        else {
                            probability += log(1 / 
                                static_cast<double>(input.num_posts()));
                        }
                    }
                }
                temp[*x] = probability;
            }
            probabilitiesfortest.push_back(temp);
        }
    }
    pair<string, double> maximume(vector<map<string, double>>& probabilities, 
        const int& iter) {
        pair<string, double> maximum;
        maximum.first = probabilities[iter].begin()->first;
        maximum.second = probabilities[iter].begin()->second;
        for (auto iterator = probabilities[iter].begin(); 
            iterator != probabilities[iter].end(); iterator++)
        {
            if (iterator->second > maximum.second)
            {
                maximum.first = iterator->first;
                maximum.second = iterator->second;
            }
            else {

            }
        }
        return maximum;
    }
    void printtest(vector<map<string, double>>& probs)
    {
        cout << endl << endl << "test data:" << endl;
        int temp1 = 0;
        for (int i = 0; i < num_posts(); i++)
        {
            pair<string, double> temp;
            temp = maximume(probs, i);
            cout << "  correct = " << returnTag(i) << ", predicted = ";
            cout << temp.first << ", log-probability score = "
                << temp.second << endl <<"  content = " << returnPost(i) 
                << endl << endl;
            if (returnTag(i) == temp.first)
                temp1++;
        }
        cout << "performance: " << temp1 << " / " << num_posts()
            << " posts predicted correctly" << endl;
    }
    //construct train and test functions 
    void train(csvstream& data_in, int argc) {
        map<string, string> temp;
        readin(data_in);
        count();
    }
    void trainingprint()
    {
       
        printtrainingdata();
        for (auto it = uniquetags.begin(); it != uniquetags.end(); it++)
        {
            double temp = log(num_posts_withC[*it] / 
                static_cast<double>(num_posts()));
            cout << "  " << *it << ", " << num_posts_withC[*it]
                << " examples, log-prior = "
                << temp << endl;
        }
        trainingprintparameters();

    }
    void printtrainingdata() {
        cout << "training data:" << endl;
        for (int i = 0; i < num_posts(); i++) {
            cout << "  label = " << tag[i] << ", content = ";
            cout << post[i] << endl;
       }


        cout << "trained on " << num_posts() << " examples" << endl;
        cout << "vocabulary size = " << uniquewords.size() << endl << endl;
        cout << "classes:" << endl;
    }
    void trainingprintparameters() {
        cout << "classifier parameters: " << endl;

        for (auto it = num_posts_with_wc.begin(); it !=
            num_posts_with_wc.end(); it++)
        {
            double temp = log(num_posts_with_wc[{it->first.first, it->first.second}] /
                static_cast<double>(num_posts_withC[it->first.first]));

            if (num_posts_with_wc[{it->first.first, it->first.second}] > 0) {
                cout << "  " << it->first.first << ":" << it->first.second << 
                    ", count = "
                    << num_posts_with_wc[{it->first.first, it->first.second}]
                    << ", log-likelihood = " << temp << endl;
            }
        }
    }
    void test(csvstream& data_in) {
        readin(data_in);
        count();
    }
};
pair<string, double> maximum(vector<map<string, double>>& probabilities, 
    const int& iter) {
    pair<string, double> maximum;
    maximum.first = probabilities[iter].begin()->first;
    maximum.second = probabilities[iter].begin()->second;
    for (auto iterator = probabilities[iter].begin(); 
        iterator != probabilities[iter].end(); iterator++)
    {
        if (iterator->second > maximum.second)
        {
            maximum.first = iterator->first;
            maximum.second = iterator->second;
        }
        else {

        }
    }
    return maximum;
}
void info(classifier& testing,
    vector<map<string, double>>& probs)
{
    cout << endl <<"test data:" << endl;
    int correct = 0;
    for (int i = 0; i < testing.num_posts(); i++)
    {
        pair<string, double> temp;
        cout << "  correct = " << testing.returnTag(i) << ", predicted = ";
        temp = maximum(probs, i);
        cout << temp.first << ", log-probability score = "
            << temp.second << endl << "  content = " << 
            testing.returnPost(i) << endl << endl;
        if (testing.returnTag(i) == temp.first)
            correct++;
    }
    cout << "performance: " << correct << " / " << testing.num_posts()
        << " posts predicted correctly" << endl;
}

    int main(int argc, char* argv[])
    {
        cout.precision(3);
        ifstream train;
        ifstream test;
        train.open(argv[1]);
        test.open(argv[2]);
        string temp;
        if (argv[3])
            temp = argv[3];

        if ((argc != 3 && argc != 4) || (temp != "--debug" && temp != "")) {
            cout << "Usage: main.exe TRAIN_FILE TEST_FILE [--debug]" << endl;
            return 1;
        }

        if (!train.is_open()) {
            cout << "Error opening file: " << argv[1] << endl;
            return 1;
        }

        if (!test.is_open()) {
            cout << "Error opening file: " << argv[2] << endl;
            return 1;
        }

        csvstream data_in(argv[1]);
        csvstream test_data(argv[2]);
        classifier training;
        classifier testing;
        vector<map<string, double>> probabilitycontainer;
        training.train(data_in, argc);
        testing.readin(test_data);
        if (temp == "--debug")
            training.trainingprint();
        else
            cout << "trained on " << training.num_posts() << 
            " examples" << endl;
        //we are experiencing an issue with getProbabilitiesTest and printtest
        testing.getProbabilitiesTest(training, probabilitycontainer);
        info(testing, probabilitycontainer);
    }
