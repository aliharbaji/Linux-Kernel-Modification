#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/list.h>


asmlinkage long sys_hello(void) {
	printk("Hello, World!\n");
	return 0;
}

// sets the weight of the current process
asmlinkage long sys_set_weight(int weight) {
	if(weight < 0){ // Failure
		return -EINVAL;
	}
	// Success
	current->weight = weight;
	return 0;
}
// get the weight of the current process
asmlinkage long sys_get_weight(void) {
	//printk("sys_get_weight: SUCCESS TEST\n");
	return current->weight;
}


long get_sum_of_path_to_child_aux(struct task_struct *task, pid_t target, unsigned int sum){
    struct list_head *list;

    int current_sum = sum + task->weight;
    if(task->pid == target){
        return current_sum;
    }

    list_for_each(list, &task->children){
        struct task_struct *child = list_entry(list, struct task_struct, sibling);
        int result = get_sum_of_path_to_child_aux(child, target, current_sum);
        if(result != -1){
            return result;
        }
    }
    return -1;
}

bool contains_aux(struct task_struct* task, pid_t target){
    struct list_head *list;
    
    if (task->pid == target) {
        return true;  // Found the target
    }
    
    list_for_each(list, &task->children){
        struct task_struct *child = list_entry(list, struct task_struct, sibling);
        if (contains_aux(child, target)) {
            return true;
        }
    }
    return false;
}

bool contains(pid_t target){
    return contains_aux(current, target);
}


asmlinkage long sys_get_path_sum(pid_t target){
    if (target <= 0) {
        return -ECHILD;  // Invalid PID
    }
    
    if (target == current->pid) {
        return current->weight;  // Special case: current process
    }
    
    if (!contains(target)) {
        return -ECHILD;  // Not a descendant
    }
    return get_sum_of_path_to_child_aux(current, target, 0);
}

asmlinkage long sys_get_heaviest_sibling(void) {
    unsigned int heaviest_weight = current->weight;  // Initial value for heaviest_weight
    pid_t heaviest_pid = current->pid;           // Initial value for heaviest_pid

    struct list_head *head_list;
    list_for_each(head_list, &current->parent->children) {
        struct task_struct *task = list_entry(head_list, struct task_struct, sibling);
        if (task->weight > heaviest_weight) {
            heaviest_weight = task->weight;
            heaviest_pid = task->pid;
        } if(task->weight == heaviest_weight){
            if(task->pid < heaviest_pid){
                heaviest_pid = task->pid;
            }
        }
    }

    return heaviest_pid;
}



//

//
//asmlinkage long sys_get_path_sum(pid_t target){
//
//	if(!contains(target)){ // if it is not a child of the current task
//		printk("sys_get_path_sum: NOT A CHILD\n");
//        return -ECHILD;
//    }
//    printk("sys_get_path_sum: SUCCESS IS A CHILD AND SUM WILL BE RETd\n");
//    return get_sum_of_path_to_child(target);
//}
//
//void get_heaviest_sibling_aux(unsigned int* heaviest_weight, struct task_struct* task, pid_t* heaviest_pid){
//    struct list_head *list;
//    list_for_each(list, &task->parent->children){
//        struct task_struct *sibling = list_entry(list, struct task_struct, sibling);
//        if(sibling->weight > *heaviest_weight){
//            *heaviest_weight = sibling->weight;
//            *heaviest_pid = sibling->pid;
//        }
//        get_heaviest_sibling_aux(heaviest_weight, sibling, heaviest_pid);
//    }
//}
//
//asmlinkage long sys_get_heaviest_sibling(void) {
//    unsigned int heaviest_weight = 0;  // Variable declaration moved to the beginning
//    pid_t heaviest_pid = -1;           // Initial value for heaviest_pid
//
//    printk("sys_get_heaviest_sibling: SUCCESS\n");
//
//    get_heaviest_sibling_aux(&heaviest_weight, current, &heaviest_pid);
//
//    return heaviest_pid;
//}

